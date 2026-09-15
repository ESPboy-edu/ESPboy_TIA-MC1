#pragma once

#define FLG_S  0x80
#define FLG_Z 0x40
#define FLG_Y 0x20
#define FLG_AC  0x10
#define FLG_X 0x08
#define FLG_P 0x04
#define FLG_N 0x02
#define FLG_CY  0x01

namespace CPU8080
{
  unsigned char(*mem_rd)(unsigned short int adr) = 0;
  void(*mem_wr)(unsigned short int adr, unsigned char val) = 0;
  unsigned char(*port_rd)(unsigned char adr) = 0;
  void(*port_wr)(unsigned char adr, unsigned char val) = 0;

  int time;
  int run_time;

  unsigned char A, F, B, C, D, E, H, L;
  unsigned short int PC, SP;
  bool IR_INTE, IR_HALT;

  unsigned char FLG_SZP[256];

  inline unsigned short int ARG16() {
    unsigned short int temp = mem_rd(PC) + (mem_rd(PC + 1) << 8);
    PC += 2;
    return temp;
  }

  inline void NOOP(void) { time += 4; }
  inline void CMA(void) { A ^= 0xff; time += 4; }

  inline void DAA(void) {
    unsigned char temp = A;
    if (F & FLG_N) {
      if ((F & FLG_AC) || ((A & 0x0f) > 9)) temp -= 0x06;
      if ((F & FLG_CY) || (A > 0x99)) temp -= 0x60;
    } else {
      if ((F & FLG_AC) || ((A & 0x0f) > 9)) temp += 0x06;
      if ((F & FLG_CY) || (A > 0x99)) temp += 0x60;
    }
    temp &= 0xff;
    F = (F & (FLG_CY | FLG_N)) | (A > 0x99 ? 1 : 0) | ((A ^ temp) & FLG_AC) | FLG_SZP[temp];
    A = temp;
    time += 4;
  }

  inline void HALT(void) {
    IR_HALT = true;
    time += 7;
    time += int(((run_time - time) + 3) / 4) * 4;
  }

  inline void DI(void) { IR_INTE = false; time += 4; }
  inline void EI(void) { IR_INTE = true; time += 4; }

  inline void IN_A(void) { A = port_rd(mem_rd(PC++)); time += 10; }
  inline void OUT_A(void) { port_wr(mem_rd(PC++), A); time += 10; }

  inline void CMC(void) { F ^= FLG_CY; time += 4; }
  inline void STC(void) { F |= FLG_CY; time += 4; }

  inline void STAX_BC(void) { mem_wr(C + (B << 8), A); time += 7; }
  inline void STAX_DE(void) { mem_wr(E + (D << 8), A); time += 7; }

  inline void LDAX_BC(void) { A = mem_rd(C + (B << 8)); time += 7; }
  inline void LDAX_DE(void) { A = mem_rd(E + (D << 8)); time += 7; }

  inline void MVI_A(void) { A = mem_rd(PC++); time += 7; }
  inline void MVI_B(void) { B = mem_rd(PC++); time += 7; }
  inline void MVI_C(void) { C = mem_rd(PC++); time += 7; }
  inline void MVI_D(void) { D = mem_rd(PC++); time += 7; }
  inline void MVI_E(void) { E = mem_rd(PC++); time += 7; }
  inline void MVI_H(void) { H = mem_rd(PC++); time += 7; }
  inline void MVI_L(void) { L = mem_rd(PC++); time += 7; }

  inline void MVI_M(void) { mem_wr(L + (H << 8), mem_rd(PC++)); time += 10; }

  inline void LDA(void) { A = mem_rd(ARG16()); time += 13; }
  inline void STA(void) { mem_wr(ARG16(), A); time += 13; }

  inline void SHLD(void) {
    unsigned short int temp = ARG16();
    mem_wr(temp + 0, L);
    mem_wr(temp + 1, H);
    time += 16;
  }

  inline void LHLD(void) {
    unsigned short int temp = ARG16();
    L = mem_rd(temp + 0);
    H = mem_rd(temp + 1);
    time += 16;
  }

  inline void MOV_AA(void) { A = A; time += 5; }
  inline void MOV_AB(void) { A = B; time += 5; }
  inline void MOV_AC(void) { A = C; time += 5; }
  inline void MOV_AD(void) { A = D; time += 5; }
  inline void MOV_AE(void) { A = E; time += 5; }
  inline void MOV_AH(void) { A = H; time += 5; }
  inline void MOV_AL(void) { A = L; time += 5; }

  inline void MOV_BA(void) { B = A; time += 5; }
  inline void MOV_BB(void) { B = B; time += 5; }
  inline void MOV_BC(void) { B = C; time += 5; }
  inline void MOV_BD(void) { B = D; time += 5; }
  inline void MOV_BE(void) { B = E; time += 5; }
  inline void MOV_BH(void) { B = H; time += 5; }
  inline void MOV_BL(void) { B = L; time += 5; }

  inline void MOV_CA(void) { C = A; time += 5; }
  inline void MOV_CB(void) { C = B; time += 5; }
  inline void MOV_CC(void) { C = C; time += 5; }
  inline void MOV_CD(void) { C = D; time += 5; }
  inline void MOV_CE(void) { C = E; time += 5; }
  inline void MOV_CH(void) { C = H; time += 5; }
  inline void MOV_CL(void) { C = L; time += 5; }

  inline void MOV_DA(void) { D = A; time += 5; }
  inline void MOV_DB(void) { D = B; time += 5; }
  inline void MOV_DC(void) { D = C; time += 5; }
  inline void MOV_DD(void) { D = D; time += 5; }
  inline void MOV_DE(void) { D = E; time += 5; }
  inline void MOV_DH(void) { D = H; time += 5; }
  inline void MOV_DL(void) { D = L; time += 5; }

  inline void MOV_EA(void) { E = A; time += 5; }
  inline void MOV_EB(void) { E = B; time += 5; }
  inline void MOV_EC(void) { E = C; time += 5; }
  inline void MOV_ED(void) { E = D; time += 5; }
  inline void MOV_EE(void) { E = E; time += 5; }
  inline void MOV_EH(void) { E = H; time += 5; }
  inline void MOV_EL(void) { E = L; time += 5; }

  inline void MOV_HA(void) { H = A; time += 5; }
  inline void MOV_HB(void) { H = B; time += 5; }
  inline void MOV_HC(void) { H = C; time += 5; }
  inline void MOV_HD(void) { H = D; time += 5; }
  inline void MOV_HE(void) { H = E; time += 5; }
  inline void MOV_HH(void) { H = H; time += 5; }
  inline void MOV_HL(void) { H = L; time += 5; }

  inline void MOV_LA(void) { L = A; time += 5; }
  inline void MOV_LB(void) { L = B; time += 5; }
  inline void MOV_LC(void) { L = C; time += 5; }
  inline void MOV_LD(void) { L = D; time += 5; }
  inline void MOV_LE(void) { L = E; time += 5; }
  inline void MOV_LH(void) { L = H; time += 5; }
  inline void MOV_LL(void) { L = L; time += 5; }

  inline void MOV_AM(void) { A = mem_rd(L + (H << 8)); time += 7; }
  inline void MOV_BM(void) { B = mem_rd(L + (H << 8)); time += 7; }
  inline void MOV_CM(void) { C = mem_rd(L + (H << 8)); time += 7; }
  inline void MOV_DM(void) { D = mem_rd(L + (H << 8)); time += 7; }
  inline void MOV_EM(void) { E = mem_rd(L + (H << 8)); time += 7; }
  inline void MOV_HM(void) { H = mem_rd(L + (H << 8)); time += 7; }
  inline void MOV_LM(void) { L = mem_rd(L + (H << 8)); time += 7; }

  inline void MOV_WA(void) { mem_wr(L + (H << 8), A); time += 7; }
  inline void MOV_WB(void) { mem_wr(L + (H << 8), B); time += 7; }
  inline void MOV_WC(void) { mem_wr(L + (H << 8), C); time += 7; }
  inline void MOV_WD(void) { mem_wr(L + (H << 8), D); time += 7; }
  inline void MOV_WE(void) { mem_wr(L + (H << 8), E); time += 7; }
  inline void MOV_WH(void) { mem_wr(L + (H << 8), H); time += 7; }
  inline void MOV_WL(void) { mem_wr(L + (H << 8), L); time += 7; }

  inline void LXI_BC(void) { C = mem_rd(PC++); B = mem_rd(PC++); time += 10; }
  inline void LXI_DE(void) { E = mem_rd(PC++); D = mem_rd(PC++); time += 10; }
  inline void LXI_HL(void) { L = mem_rd(PC++); H = mem_rd(PC++); time += 10; }
  inline void LXI_SP(void) { SP = ARG16(); time += 10; }

  inline void XCHG(void) {
    unsigned char temp = E; E = L; L = temp;
    temp = H; H = D; D = temp;
    time += 4;
  }

  inline void INCDEC_FLAGS(unsigned char reg) {
    F = (F & ~(FLG_S | FLG_Z | FLG_P | FLG_AC)) | FLG_SZP[reg] | (((reg & 0x0f) == 0x00) ? FLG_AC : 0);
  }

  inline void INR_A(void) { A = (A + 1) & 255; INCDEC_FLAGS(A); time += 5; }
  inline void INR_B(void) { B = (B + 1) & 255; INCDEC_FLAGS(B); time += 5; }
  inline void INR_C(void) { C = (C + 1) & 255; INCDEC_FLAGS(C); time += 5; }
  inline void INR_D(void) { D = (D + 1) & 255; INCDEC_FLAGS(D); time += 5; }
  inline void INR_E(void) { E = (E + 1) & 255; INCDEC_FLAGS(E); time += 5; }
  inline void INR_H(void) { H = (H + 1) & 255; INCDEC_FLAGS(H); time += 5; }
  inline void INR_L(void) { L = (L + 1) & 255; INCDEC_FLAGS(L); time += 5; }

  inline void DCR_A(void) { A = (A - 1) & 255; INCDEC_FLAGS(A); time += 5; }
  inline void DCR_B(void) { B = (B - 1) & 255; INCDEC_FLAGS(B); time += 5; }
  inline void DCR_C(void) { C = (C - 1) & 255; INCDEC_FLAGS(C); time += 5; }
  inline void DCR_D(void) { D = (D - 1) & 255; INCDEC_FLAGS(D); time += 5; }
  inline void DCR_E(void) { E = (E - 1) & 255; INCDEC_FLAGS(E); time += 5; }
  inline void DCR_H(void) { H = (H - 1) & 255; INCDEC_FLAGS(H); time += 5; }
  inline void DCR_L(void) { L = (L - 1) & 255; INCDEC_FLAGS(L); time += 5; }

  inline void INR_M(void) {
    unsigned short int adr = L + (H << 8);
    unsigned char val = (mem_rd(adr) + 1) & 255;
    INCDEC_FLAGS(val);
    mem_wr(adr, val);
    time += 10;
  }

  inline void DCR_M(void) {
    unsigned short int adr = L + (H << 8);
    unsigned char val = (mem_rd(adr) - 1) & 255;
    INCDEC_FLAGS(val);
    mem_wr(adr, val);
    time += 10;
  }

  inline void INX_BC(void) { C = (C + 1) & 255; if (!C) B = (B + 1) & 255; time += 5; }
  inline void INX_DE(void) { E = (E + 1) & 255; if (!E) D = (D + 1) & 255; time += 5; }
  inline void INX_HL(void) { L = (L + 1) & 255; if (!L) H = (H + 1) & 255; time += 5; }
  inline void INX_SP(void) { SP = (SP + 1) & 0xffff; time += 5; }

  inline void DEX_BC(void) { C = (C - 1) & 255; if (C == 255) B = (B - 1) & 255; time += 5; }
  inline void DEX_DE(void) { E = (E - 1) & 255; if (E == 255) D = (D - 1) & 255; time += 5; }
  inline void DEX_HL(void) { L = (L - 1) & 255; if (L == 255) H = (H - 1) & 255; time += 5; }
  inline void DEX_SP(void) { SP = (SP - 1) & 0xffff; time += 5; }

  inline void _ADD(unsigned char reg) {
    unsigned short int t1 = reg;
    unsigned short int t2 = A + reg;
    F = FLG_SZP[t2 & 0xff] | ((t2 >> 8) & FLG_CY) | ((A ^ t2 ^ t1) & FLG_AC) | (((t1 ^ A ^ FLG_S) & (t1 ^ t2) & FLG_S) >> 5);
    A = t2 & 0xff;
  }

  inline void _ADC(unsigned char reg) {
    unsigned short int t1 = reg;
    unsigned short int t2 = A + reg + (F & FLG_CY);
    F = FLG_SZP[t2 & 0xff] | ((t2 >> 8) & FLG_CY) | ((A ^ t2 ^ t1) & FLG_AC) | (((t1 ^ A ^ FLG_S) & (t1 ^ t2) & FLG_S) >> 5);
    A = t2 & 0xff;
  }

  inline void _SUB(unsigned char reg) {
    unsigned short int t1 = reg;
    unsigned short int t2 = A - t1;
    F = FLG_SZP[t2 & 0xff] | ((t2 >> 8) & FLG_CY) | ((A ^ t2 ^ t1) & FLG_AC) | (((t1 ^ A) & (A ^ t2) & FLG_S) >> 5);
    A = t2 & 0xff;
  }

  inline void _SBB(unsigned char reg) {
    unsigned short int t1 = reg;
    unsigned short int t2 = A - t1 - (F & FLG_CY);
    F = FLG_SZP[t2 & 0xff] | ((t2 >> 8) & FLG_CY) | ((A ^ t2 ^ t1) & FLG_AC) | (((t1 ^ A) & (A ^ t2) & FLG_S) >> 5);
    A = t2 & 0xff;
  }

  inline void ADD_A(void) { _ADD(A); time += 4; }
  inline void ADD_B(void) { _ADD(B); time += 4; }
  inline void ADD_C(void) { _ADD(C); time += 4; }
  inline void ADD_D(void) { _ADD(D); time += 4; }
  inline void ADD_E(void) { _ADD(E); time += 4; }
  inline void ADD_H(void) { _ADD(H); time += 4; }
  inline void ADD_L(void) { _ADD(L); time += 4; }
  inline void ADD_M(void) { _ADD(mem_rd(L + (H << 8))); time += 7; }
  inline void ADI(void) { _ADD(mem_rd(PC++)); time += 7; }

  inline void ADC_A(void) { _ADC(A); time += 4; }
  inline void ADC_B(void) { _ADC(B); time += 4; }
  inline void ADC_C(void) { _ADC(C); time += 4; }
  inline void ADC_D(void) { _ADC(D); time += 4; }
  inline void ADC_E(void) { _ADC(E); time += 4; }
  inline void ADC_H(void) { _ADC(H); time += 4; }
  inline void ADC_L(void) { _ADC(L); time += 4; }
  inline void ADC_M(void) { _ADC(mem_rd(L + (H << 8))); time += 7; }
  inline void ACI(void) { _ADC(mem_rd(PC++)); time += 7; }

  inline void SUB_A(void) { _SUB(A); time += 4; }
  inline void SUB_B(void) { _SUB(B); time += 4; }
  inline void SUB_C(void) { _SUB(C); time += 4; }
  inline void SUB_D(void) { _SUB(D); time += 4; }
  inline void SUB_E(void) { _SUB(E); time += 4; }
  inline void SUB_H(void) { _SUB(H); time += 4; }
  inline void SUB_L(void) { _SUB(L); time += 4; }
  inline void SUB_M(void) { _SUB(mem_rd(L + (H << 8))); time += 7; }
  inline void SUI(void) { _SUB(mem_rd(PC++)); time += 7; }

  inline void SBB_A(void) { _SBB(A); time += 4; }
  inline void SBB_B(void) { _SBB(B); time += 4; }
  inline void SBB_C(void) { _SBB(C); time += 4; }
  inline void SBB_D(void) { _SBB(D); time += 4; }
  inline void SBB_E(void) { _SBB(E); time += 4; }
  inline void SBB_H(void) { _SBB(H); time += 4; }
  inline void SBB_L(void) { _SBB(L); time += 4; }
  inline void SBB_M(void) { _SBB(mem_rd(L + (H << 8))); time += 7; }
  inline void SBI(void) { _SBB(mem_rd(PC++)); time += 7; }

  inline void _DAD(unsigned short int pair) {
    unsigned int value = L + (H << 8);
    unsigned int temp = value + pair;
    F = (F & ~(FLG_CY | FLG_AC)) | (((value ^ temp ^ pair) >> 8) & FLG_AC) | ((temp >> 16) & FLG_CY);
    L = temp & 0xff;
    H = (temp >> 8) & 0xff;
    time += 10;
  }

  inline void DAD_BC(void) { _DAD(C + (B << 8)); }
  inline void DAD_DE(void) { _DAD(E + (D << 8)); }
  inline void DAD_HL(void) { _DAD(L + (H << 8)); }
  inline void DAD_SP(void) { _DAD(SP); }

  inline void _ANA(unsigned char reg) {
    unsigned char temp = reg;
    F = (F & ~(FLG_S | FLG_Z | FLG_P | FLG_CY | FLG_AC)) | (((A | temp) & 8) ? FLG_AC : 0);
    A &= temp;
    F |= FLG_SZP[A];
  }

  inline void _XRA(unsigned char reg) {
    A ^= reg;
    F = (F & ~(FLG_S | FLG_Z | FLG_P | FLG_CY | FLG_AC)) | FLG_SZP[A];
  }

  inline void _ORA(unsigned char reg) {
    A |= reg;
    F = (F & ~(FLG_S | FLG_Z | FLG_P | FLG_CY | FLG_AC)) | FLG_SZP[A];
  }

  inline void _CMP(unsigned char reg) {
    unsigned char temp = (A - reg) & 0xff;
    F = (F & ~(FLG_S | FLG_Z | FLG_P | FLG_CY | FLG_AC)) | FLG_SZP[temp] | ((temp > A) ? FLG_CY : 0) | (((temp & 0x0f) > (A & 0x0f)) ? FLG_AC : 0);
  }

  inline void ANA_A(void) { _ANA(A); time += 4; }
  inline void ANA_B(void) { _ANA(B); time += 4; }
  inline void ANA_C(void) { _ANA(C); time += 4; }
  inline void ANA_D(void) { _ANA(D); time += 4; }
  inline void ANA_E(void) { _ANA(E); time += 4; }
  inline void ANA_H(void) { _ANA(H); time += 4; }
  inline void ANA_L(void) { _ANA(L); time += 4; }
  inline void ANA_M(void) { _ANA(mem_rd(L + (H << 8))); time += 7; }
  inline void ANI(void) { _ANA(mem_rd(PC++)); time += 7; }

  inline void XRA_A(void) { _XRA(A); time += 4; }
  inline void XRA_B(void) { _XRA(B); time += 4; }
  inline void XRA_C(void) { _XRA(C); time += 4; }
  inline void XRA_D(void) { _XRA(D); time += 4; }
  inline void XRA_E(void) { _XRA(E); time += 4; }
  inline void XRA_H(void) { _XRA(H); time += 4; }
  inline void XRA_L(void) { _XRA(L); time += 4; }
  inline void XRA_M(void) { _XRA(mem_rd(L + (H << 8))); time += 7; }
  inline void XRI(void) { _XRA(mem_rd(PC++)); time += 7; }

  inline void ORA_A(void) { _ORA(A); time += 4; }
  inline void ORA_B(void) { _ORA(B); time += 4; }
  inline void ORA_C(void) { _ORA(C); time += 4; }
  inline void ORA_D(void) { _ORA(D); time += 4; }
  inline void ORA_E(void) { _ORA(E); time += 4; }
  inline void ORA_H(void) { _ORA(H); time += 4; }
  inline void ORA_L(void) { _ORA(L); time += 4; }
  inline void ORA_M(void) { _ORA(mem_rd(L + (H << 8))); time += 7; }
  inline void ORI(void) { _ORA(mem_rd(PC++)); time += 7; }

  inline void CMP_A(void) { _CMP(A); time += 4; }
  inline void CMP_B(void) { _CMP(B); time += 4; }
  inline void CMP_C(void) { _CMP(C); time += 4; }
  inline void CMP_D(void) { _CMP(D); time += 4; }
  inline void CMP_E(void) { _CMP(E); time += 4; }
  inline void CMP_H(void) { _CMP(H); time += 4; }
  inline void CMP_L(void) { _CMP(L); time += 4; }
  inline void CMP_M(void) { _CMP(mem_rd(L + (H << 8))); time += 7; }
  inline void CPI(void) { _CMP(mem_rd(PC++)); time += 7; }

  inline void RLC(void) {
    unsigned char temp = A;
    A = (A << 1) & 0xff;
    if (temp & 0x80) { A |= 0x01; F |= FLG_CY; } else F &= ~FLG_CY;
    time += 4;
  }

  inline void RRC(void) {
    unsigned char temp = A;
    A = (A >> 1) & 0xff;
    if (temp & 0x01) { A |= 0x80; F |= FLG_CY; } else F &= ~FLG_CY;
    time += 4;
  }

  inline void RAL(void) {
    unsigned char temp = A;
    A = (A << 1) & 0xff;
    if (F & FLG_CY) A |= 0x01; if (temp & 0x80) F |= FLG_CY; else F &= ~FLG_CY;
    time += 4;
  }

  inline void RAR(void) {
    unsigned char temp = A;
    A = (A >> 1) & 0xff;
    if (F & FLG_CY) A |= 0x80; if (temp & 0x01) F |= FLG_CY; else F &= ~FLG_CY;
    time += 4;
  }

  inline void _PUSH(unsigned short int pair) {
    mem_wr(SP - 1, (pair >> 8) & 0xff);
    mem_wr(SP - 2, pair & 0xff);
    SP -= 2;
  }

  inline void PUSH_AF(void) { _PUSH(F + (A << 8)); time += 11; }
  inline void PUSH_BC(void) { _PUSH(C + (B << 8)); time += 11; }
  inline void PUSH_DE(void) { _PUSH(E + (D << 8)); time += 11; }
  inline void PUSH_HL(void) { _PUSH(L + (H << 8)); time += 11; }

  inline unsigned short int _POP() {
    unsigned short int temp = mem_rd(SP) | (mem_rd(SP + 1) << 8);
    SP += 2;
    return temp;
  }

  inline void POP_AF(void) { F = mem_rd(SP++); A = mem_rd(SP++); time += 10; }
  inline void POP_BC(void) { C = mem_rd(SP++); B = mem_rd(SP++); time += 10; }
  inline void POP_DE(void) { E = mem_rd(SP++); D = mem_rd(SP++); time += 10; }
  inline void POP_HL(void) { L = mem_rd(SP++); H = mem_rd(SP++); time += 10; }

  inline void XTHL(void) {
    unsigned short int adr1 = SP;
    unsigned short int adr2 = (SP + 1) & 0xffff;
    unsigned char temp;
    temp = mem_rd(adr1); mem_wr(adr1, L); L = temp;
    temp = mem_rd(adr2); mem_wr(adr2, H); H = temp;
    time += 18;
  }

  inline void SPHL(void) { SP = L + (H << 8); time += 5; }

  inline void _JMPC(unsigned char cond) {
    unsigned short int temp = ARG16();
    if (cond) PC = temp;
    time += 10;
  }

  inline void JMP(void) { _JMPC(1); }
  inline void JMP_Z(void) { _JMPC(F & FLG_Z); }
  inline void JMP_C(void) { _JMPC(F & FLG_CY); }
  inline void JMP_P(void) { _JMPC(F & FLG_P); }
  inline void JMP_S(void) { _JMPC(F & FLG_S); }
  inline void JMP_NZ(void) { _JMPC(~F & FLG_Z); }
  inline void JMP_NC(void) { _JMPC(~F & FLG_CY); }
  inline void JMP_NP(void) { _JMPC(~F & FLG_P); }
  inline void JMP_NS(void) { _JMPC(~F & FLG_S); }

  inline void _CALLC(unsigned char cond) {
    unsigned short int temp = ARG16();
    if (cond) { _PUSH(PC); PC = temp; time += 17; return; }
    time += 11;
  }

  inline void CALL(void) { _CALLC(1); }
  inline void CALL_Z(void) { _CALLC(F & FLG_Z); }
  inline void CALL_C(void) { _CALLC(F & FLG_CY); }
  inline void CALL_P(void) { _CALLC(F & FLG_P); }
  inline void CALL_S(void) { _CALLC(F & FLG_S); }
  inline void CALL_NZ(void) { _CALLC(~F & FLG_Z); }
  inline void CALL_NC(void) { _CALLC(~F & FLG_CY); }
  inline void CALL_NP(void) { _CALLC(~F & FLG_P); }
  inline void CALL_NS(void) { _CALLC(~F & FLG_S); }

  inline void _RETC(unsigned char cond) {
    if (cond) { PC = _POP(); time += 11; return; }
    time += 5;
  }

  inline void RET(void) { _RETC(1); }
  inline void RET_Z(void) { _RETC(F & FLG_Z); }
  inline void RET_C(void) { _RETC(F & FLG_CY); }
  inline void RET_P(void) { _RETC(F & FLG_P); }
  inline void RET_S(void) { _RETC(F & FLG_S); }
  inline void RET_NZ(void) { _RETC(~F & FLG_Z); }
  inline void RET_NC(void) { _RETC(~F & FLG_CY); }
  inline void RET_NP(void) { _RETC(~F & FLG_P); }
  inline void RET_NS(void) { _RETC(~F & FLG_S); }

  inline void RST_00(void) { _PUSH(PC); PC = 0x00; time += 11; }
  inline void RST_08(void) { _PUSH(PC); PC = 0x08; time += 11; }
  inline void RST_10(void) { _PUSH(PC); PC = 0x10; time += 11; }
  inline void RST_18(void) { _PUSH(PC); PC = 0x18; time += 11; }
  inline void RST_20(void) { _PUSH(PC); PC = 0x20; time += 11; }
  inline void RST_28(void) { _PUSH(PC); PC = 0x28; time += 11; }
  inline void RST_30(void) { _PUSH(PC); PC = 0x30; time += 11; }
  inline void RST_38(void) { _PUSH(PC); PC = 0x38; time += 11; }

  inline void PCHL() { PC = L + (H << 8); time += 5; }

  void(*jump_table[256])(void) = {
    NOOP, LXI_BC, STAX_BC, INX_BC, INR_B, DCR_B, MVI_B, RLC,
    NOOP, DAD_BC, LDAX_BC, DEX_BC, INR_C, DCR_C, MVI_C, RRC,
    NOOP, LXI_DE, STAX_DE, INX_DE, INR_D, DCR_D, MVI_D, RAL,
    NOOP, DAD_DE, LDAX_DE, DEX_DE, INR_E, DCR_E, MVI_E, RAR,
    NOOP, LXI_HL, SHLD, INX_HL, INR_H, DCR_H, MVI_H, DAA,
    NOOP, DAD_HL, LHLD, DEX_HL, INR_L, DCR_L, MVI_L, CMA,
    NOOP, LXI_SP, STA, INX_SP, INR_M, DCR_M, MVI_M, STC,
    NOOP, DAD_SP, LDA, DEX_SP, INR_A, DCR_A, MVI_A, CMC,

    MOV_BB, MOV_BC, MOV_BD, MOV_BE, MOV_BH, MOV_BL, MOV_BM, MOV_BA,
    MOV_CB, MOV_CC, MOV_CD, MOV_CE, MOV_CH, MOV_CL, MOV_CM, MOV_CA,
    MOV_DB, MOV_DC, MOV_DD, MOV_DE, MOV_DH, MOV_DL, MOV_DM, MOV_DA,
    MOV_EB, MOV_EC, MOV_ED, MOV_EE, MOV_EH, MOV_EL, MOV_EM, MOV_EA,
    MOV_HB, MOV_HC, MOV_HD, MOV_HE, MOV_HH, MOV_HL, MOV_HM, MOV_HA,
    MOV_LB, MOV_LC, MOV_LD, MOV_LE, MOV_LH, MOV_LL, MOV_LM, MOV_LA,
    MOV_WB, MOV_WC, MOV_WD, MOV_WE, MOV_WH, MOV_WL, HALT, MOV_WA,
    MOV_AB, MOV_AC, MOV_AD, MOV_AE, MOV_AH, MOV_AL, MOV_AM, MOV_AA,

    ADD_B, ADD_C, ADD_D, ADD_E, ADD_H, ADD_L, ADD_M, ADD_A,
    ADC_B, ADC_C, ADC_D, ADC_E, ADC_H, ADC_L, ADC_M, ADC_A,
    SUB_B, SUB_C, SUB_D, SUB_E, SUB_H, SUB_L, SUB_M, SUB_A,
    SBB_B, SBB_C, SBB_D, SBB_E, SBB_H, SBB_L, SBB_M, SBB_A,
    ANA_B, ANA_C, ANA_D, ANA_E, ANA_H, ANA_L, ANA_M, ANA_A,
    XRA_B, XRA_C, XRA_D, XRA_E, XRA_H, XRA_L, XRA_M, XRA_A,
    ORA_B, ORA_C, ORA_D, ORA_E, ORA_H, ORA_L, ORA_M, ORA_A,
    CMP_B, CMP_C, CMP_D, CMP_E, CMP_H, CMP_L, CMP_M, CMP_A,

    RET_NZ, POP_BC, JMP_NZ, JMP, CALL_NZ, PUSH_BC, ADI, RST_00,
    RET_Z, RET, JMP_Z, NOOP, CALL_Z, CALL, ACI, RST_08,
    RET_NC, POP_DE, JMP_NC, OUT_A, CALL_NC, PUSH_DE, SUI, RST_10,
    RET_C, NOOP, JMP_C, IN_A, CALL_C, NOOP, SBI, RST_18,
    RET_NP, POP_HL, JMP_NP, XTHL, CALL_NP, PUSH_HL, ANI, RST_20,
    RET_P, PCHL, JMP_P, XCHG, CALL_P, NOOP, XRI, RST_28,
    RET_NS, POP_AF, JMP_NS, DI, CALL_NS, PUSH_AF, ORI, RST_30,
    RET_S, SPHL, JMP_S, EI, CALL_S, NOOP, CPI, RST_38
  };

  void init(unsigned char(*_mem_rd)(unsigned short int), void(*_mem_wr)(unsigned short int, unsigned char), unsigned char(*_port_rd)(unsigned char), void(*_port_wr)(unsigned char, unsigned char)) {
    mem_rd = _mem_rd; mem_wr = _mem_wr; port_rd = _port_rd; port_wr = _port_wr;
    for (int i = 0; i < 256; ++i) {
      int val = 0;
      if (!i) val |= FLG_Z;
      if (i & 128) val |= FLG_S;
      int cnt = 0;
      for (int j = 0; j < 8; ++j) if (i & (1 << j)) ++cnt;
      if (!(cnt & 1)) val |= FLG_P;
      FLG_SZP[i] = val;
    }
  }

  void reset(void) {
    time = 0; PC = 0; IR_INTE = false; IR_HALT = false;
  }

  void IRAM_ATTR interrupt(unsigned short int adr) {
    if (!IR_INTE) return;
    IR_HALT = false;
    _PUSH(PC); PC = adr;
    time += 11;
  }

  void IRAM_ATTR run(int t) {
    if (IR_HALT) {
      time += int((t + 3) / 4) * 4;
      return;
    }
    run_time = t + time;
    while (time < run_time) {
      jump_table[mem_rd(PC++)]();
    }
  }
}
