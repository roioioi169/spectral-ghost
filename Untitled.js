// spectral-ghost-worklet.js: mono core, allocation-free in process()
const N = 1024, HOP = 256, BINS = N / 2 + 1, KT = 9, KF = 9;
const WIN = new Float32Array(N);
for (let i = 0; i < N; i++) WIN[i] = 0.5 - 0.5 * Math.cos(2 * Math.PI * i / N);
const OLA = 1 / 1.5; // Hann^2 overlap gain at 4x overlap

function fft(re, im, inv) {
  for (let i = 1, j = 0; i < N; i++) {
    let bit = N >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) {
      let t = re[i]; re[i] = re[j]; re[j] = t;
      t = im[i]; im[i] = im[j]; im[j] = t;
    }
  }
  for (let len = 2; len <= N; len <<= 1) {
    const ang = (inv ? 2 : -2) * Math.PI / len;
    const wr = Math.cos(ang), wi = Math.sin(ang);
    for (let i = 0; i < N; i += len) {
      let cr = 1, ci = 0;
      for (let k = 0; k < (len >> 1); k++) {
        const a = i + k, b = a + (len >> 1);
        const tr = re[b] * cr - im[b] * ci;
        const ti = re[b] * ci + im[b] * cr;
        re[b] = re[a] - tr; im[b] = im[a] - ti;
        re[a] += tr; im[a] += ti;
        const nr = cr * wr - ci * wi;
        ci = cr * wi + ci * wr; cr = nr;
      }
    }
  }
  if (inv) for (let i = 0; i < N; i++) { re[i] /= N; im[i] /= N; }
}

class SpectralGhost extends AudioWorkletProcessor {
  static get parameterDescriptors() {
    const d = (name, def, min, max) => ({ name, defaultValue: def, minValue: min, maxValue: max, automationRate: 'k-rate' });
    return [d('sensitivity', 1, 0.2, 4), d('bias', 0, -1, 1), d('swap', 0, 0, 1),
            d('bite', 0, 0, 1), d('drift', 0, 0, 1), d('crossMod', 0, 0, 1), d('mix', 1, 0, 1)];
  }

  constructor() {
    super();
    this.inBuf = new Float32Array(N);
    this.pend = new Float32Array(HOP); this.pendN = 0; this.readIdx = 0;
    this.readyT = new Float32Array(HOP); this.readyS = new Float32Array(HOP);
    this.olaT = new Float32Array(N); this.olaS = new Float32Array(N);
    this.hist = Array.from({ length: KT }, () => new Float32Array(BINS));
    this.hPos = 0;
    this.re = new Float32Array(N); this.im = new Float32Array(N);
    this.reT = new Float32Array(N); this.imT = new Float32Array(N);
    this.reS = new Float32Array(N); this.imS = new Float32Array(N);
    this.tmpT = new Float32Array(KT); this.tmpF = new Float32Array(KF);
    this.env = 0; this.lfo = 0;
    this.dl = new Float32Array(4096); this.dw = 0;
  }

  _frame(sens, bias) {
    const { re, im, reT, imT, reS, imS, tmpT, tmpF } = this;
    for (let i = 0; i < N; i++) { re[i] = this.inBuf[i] * WIN[i]; im[i] = 0; }
    fft(re, im, false);

    const mag = this.hist[this.hPos];
    for (let k = 0; k < BINS; k++) mag[k] = Math.hypot(re[k], im[k]);
    this.hPos = (this.hPos + 1) % KT;

    for (let k = 0; k < BINS; k++) {
      // Harmonic = median over time (sustained bins stay high across frames)
      for (let t = 0; t < KT; t++) tmpT[t] = this.hist[t][k];
      tmpT.sort();
      const H = tmpT[KT >> 1];
      // Percussive = median over frequency (transients are broadband)
      for (let j = 0; j < KF; j++) tmpF[j] = mag[Math.min(BINS - 1, Math.max(0, k - 4 + j))];
      tmpF.sort();
      const P = tmpF[KF >> 1];

      const H2 = H * H, P2 = P * P;
      let m = H2 / (H2 + P2 + 1e-12);            // harmonic soft mask
      m = (m - 0.5) * sens + 0.5 + 0.5 * bias;   // sensitivity + tonal/noise bias
      m = m < 0 ? 0 : m > 1 ? 1 : m;

      reT[k] = re[k] * (1 - m); imT[k] = im[k] * (1 - m);
      reS[k] = re[k] * m;       imS[k] = im[k] * m;
    }
    for (let k = 1; k < N / 2; k++) { // Hermitian mirror for real output
      reT[N - k] = reT[k]; imT[N - k] = -imT[k];
      reS[N - k] = reS[k]; imS[N - k] = -imS[k];
    }

    fft(reT, imT, true); fft(reS, imS, true);
    for (let i = 0; i < N; i++) {
      this.olaT[i] += reT[i] * WIN[i] * OLA;
      this.olaS[i] += reS[i] * WIN[i] * OLA;
    }
    for (let i = 0; i < HOP; i++) { this.readyT[i] = this.olaT[i]; this.readyS[i] = this.olaS[i]; }
    this.olaT.copyWithin(0, HOP); this.olaT.fill(0, N - HOP);
    this.olaS.copyWithin(0, HOP); this.olaS.fill(0, N - HOP);
  }

  process(inputs, outputs, parameters) {
    const inp = inputs[0] && inputs[0][0];
    const out = outputs[0];
    if (!out || !out[0]) return true;
    const oL = out[0], oR = out[1];
    const P = k => parameters[k][0];
    const sens = P('sensitivity'), bias = P('bias'), swap = P('swap') > 0.5;
    const bite = P('bite'), drift = P('drift'), cm = P('crossMod'), mix = P('mix');
    const drive = 1 + 9 * bite, norm = Math.tanh(drive);

    for (let n = 0; n < oL.length; n++) {
      const x = inp ? inp[n] : 0;
      const t = this.readyT[this.readIdx], s = this.readyS[this.readIdx];
      this.readIdx++;

      // Envelope follower on transient stream (instant attack, slow release)
      const a = Math.abs(t);
      this.env = a > this.env ? a : this.env * 0.9995;
      if (this.env < 1e-20) this.env = 0;        // denormal flush

      // Stream Swap routes the two streams into each other's engines
      const tIn = swap ? s : t;
      const hIn = swap ? t : s;

      // Transient FX engine: Bite (tanh shaping, normalized)
      const tOut = Math.tanh(tIn * drive) / norm;

      // Harmonic FX engine: modulated delay (pitch-drift approximation)
      this.lfo += 2 * Math.PI * 0.4 / sampleRate;
      if (this.lfo > 2 * Math.PI) this.lfo -= 2 * Math.PI;
      this.dl[this.dw] = hIn;
      let rp = this.dw - 100 * drift * (1 + Math.sin(this.lfo));
      if (rp < 0) rp += 4096;
      const i0 = rp | 0, f = rp - i0;
      const hWet = this.dl[i0 & 4095] * (1 - f) + this.dl[(i0 + 1) & 4095] * f;
      this.dw = (this.dw + 1) & 4095;

      // Cross-mod: transient energy ducks the harmonic layer
      const duck = 1 - cm * Math.min(1, this.env * 4);
      const hOut = hWet * duck;

      const wet = tOut + hOut;
      const dry = t + s;                         // perfect reconstruction (delayed by N)
      let y = mix * wet + (1 - mix) * dry;
      if (!Number.isFinite(y)) y = 0;
      if (Math.abs(y) < 1e-30) y = 0;            // denormal flush
      y = y > 1 ? 1 : y < -1 ? -1 : y;           // safety clip
      oL[n] = y; if (oR) oR[n] = y;

      // Ingest input; run a frame each full hop
      this.pend[this.pendN++] = x;
      if (this.pendN === HOP) {
        this.inBuf.copyWithin(0, HOP);
        this.inBuf.set(this.pend, N - HOP);
        this._frame(sens, bias);
        this.pendN = 0; this.readIdx = 0;
      }
    }
    return true;
  }
}
registerProcessor('spectral-ghost', SpectralGhost);
