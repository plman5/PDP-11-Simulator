#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define MEM_SIZE_IN_WORDS 32768
typedef struct ap{
  int mode;
  int reg;
  int addr;
  int value;
} addr_phrase_t;
void update_phrase_addr_values( addr_phrase_t *phrase );
void opUpdate( addr_phrase_t *phrase, int final );
int numBranches= 0;
int numBranchesTaken=0;
int instructionNum =0;
int numFetches= 0;
int readData =0;
int writeData= 0;
int mem[MEM_SIZE_IN_WORDS];
int reg[8] ={0};
int offset = 0;
addr_phrase_t src, dst;
int halt = 0;
int sign;
int final;
int n;
int z;
int v;
int c;
int ir;
int main(int argc, char* argv[]) {
  int i=0;
  int input;
  FILE* inFile = fopen("branch.in.txt","r");
  //int input = scanf("%o",&ir);
  printf("reading words in octal from stdin:\n");
  while(fscanf(inFile,"%o",&input) != EOF){
    mem[i] = input;
    printf("  0%06o\n",mem[i]);
    i++;
  }
  printf("\ninstruction trace:\n");
  while(!halt){
    printf("at 0%04o, ",reg[7]);
    ir = mem[reg[7] >> 1];
    assert( ir < 0200000 );
    reg[7] = (reg[7]+ 2) & 0177777;
    src.mode = (ir >> 9) & 07;
    src.reg = (ir >> 6) & 07;
    dst.mode = (ir >> 3) & 07;
    dst.reg = (ir) & 07;
    numFetches++;
  if (ir == 0){
      printf("halt instruction\n");
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      halt = 1;
      instructionNum++;
    }else if((ir >> 12)== 01){
      printf("mov instruction ");
      int tempReg = reg[dst.reg];
      if (dst.mode == 2){
        update_phrase_addr_values(&src);
        int tempMem = tempReg >> 1;
        mem[tempMem] = src.value;
        update_phrase_addr_values(&dst);
        numFetches-=1;
      } else {
        update_phrase_addr_values(&src);
        update_phrase_addr_values(&dst);
        dst.value = src.value;
        final = dst.value;
        opUpdate(&dst,final);
      }
      final = final & 0177777;
      sign = final & 0100000;
      if(sign){
        n = 1;
      }else{
        n = 0;
      }
      if(src.value == 0){
      z = 1;
    }else{
      z = 0;
    }
      v = 0;
      c = 0;
      printf("sm %o, sr %o dm %o dr %o\n", src.mode, src.reg, dst.mode, dst.reg);
      printf("  src.value = 0%06o\n", src.value);
      printf("  nzvc bits = 4'b%o%o%o%o\n", n, z, v, c);
      if (dst.mode == 2) {
        printf("  value 0%06o is written to 0%06o\n", src.value, tempReg);
        writeData++;
      }
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      instructionNum++;
    } else if((ir >> 12)==02){
      printf("cmp instruction ");
      update_phrase_addr_values(&src);
      update_phrase_addr_values(&dst);
      final =src.value-dst.value;
      c = (final & 0200000) >> 16;
      final = final & 0177777;
      sign = final & 0100000;
      if(sign){
        n = 1;
      }else{
        n = 0;
      }
      if(final == 0){
      z = 1;
      }else{
      z = 0;
      }
      if(((src.value & 0100000)!= (dst.value & 0100000)) && ((dst.value & 0100000) == (final & 0100000))){
        v = 1;
      }else{
        v = 0;
      }
      c = 0;
      printf("sm %o, sr %o dm %o dr %o\n", src.mode, src.reg, dst.mode, dst.reg);
      printf("  src.value = 0%06o\n", src.value);
      printf("  dst.value = 0%06o\n", dst.value);
      printf("  result    = 0%06o\n", final);
      printf("  nzvc bits = 4'b%o%o%o%o\n", n, z, v, c);
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      instructionNum++;


      }else if((ir >> 12)== 06){
      printf("add instruction ");
      update_phrase_addr_values( &src );
      update_phrase_addr_values( &dst );
      final = dst.value + src.value;
      c = (final &0200000) >>16;
      final =final & 0177777;
      sign = final& 0100000;
      if(sign){
        n = 1;
      }else{
        n = 0;
      }
      if( final == 0){
      z = 1;
      }else{
      z = 0;
      }
      if(((src.value & 0100000) == (dst.value & 0100000)) && ((src.value & 0100000) != (final & 0100000))){
        v = 1;
      }else{
        v = 0;
      }

      printf("sm %o, sr %o dm %o dr %o\n", src.mode, src.reg, dst.mode, dst.reg);
      printf("  src.value = 0%06o\n", src.value);
      printf("  dst.value = 0%06o\n", dst.value);
      printf("  result    = 0%06o\n", final);
      printf("  nzvc bits = 4'b%o%o%o%o\n", n, z, v, c);
      opUpdate(&dst,final);
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      instructionNum++;
    } else if((ir >> 12 ==016)){
      printf("sub instruction ");
      update_phrase_addr_values(&src);
      update_phrase_addr_values(&dst);
      final = dst.value - src.value;
      c=(final & 0200000) >> 16;
      final = final & 0177777;
      sign = final & 0100000;
      if(sign){
        n = 1;
      }else{
        n = 0;
      }
      if(final == 0){
      z = 1;
      }else{
      z = 0;
      }
      if(((src.value & 0100000) != (dst.value & 0100000)) && ((src.value & 0100000) ==(final    & 0100000) ) ){
        v = 1;
      }else{
        v = 0;
      }

      printf("sm %o, sr %o dm %o dr %o\n", src.mode, src.reg, dst.mode, dst.reg);
      printf("  src.value = 0%06o\n", src.value);
      printf("  dst.value = 0%06o\n", dst.value);
      printf("  result    = 0%06o\n", final);
      printf("  nzvc bits = 4'b%o%o%o%o\n", n, z, v, c);
      opUpdate(&dst,final);
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      instructionNum++;
    }else if((ir >> 9)==077){
      printf("sob instruction ");
      offset = ir & 037;
      update_phrase_addr_values(&dst);
      dst.value -= 1;
      reg[dst.reg] = dst.value;
      final = dst.value;
      printf("reg %d with offset 0%02o\n", dst.reg, offset);
      offset = offset << 24;
      offset = offset >> 24;
      if (final!= 0){
        reg[7] = (reg[7] - (offset << 1)) & 0177777;
        numBranchesTaken++;
      }
      numBranches++;
      instructionNum++;
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      }else if((ir >> 8) == 001){
      printf("br instruction ");
      offset = ir & 0377;
      printf("with offset 0%03o\n", offset);
      offset = offset << 24;
      offset = offset >> 24;
      reg[7] = (reg[7] + (offset<< 1) ) &0177777;
      numBranchesTaken++;
      numBranches++;
      instructionNum++;
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      } else if((ir >> 8)== 002){
      printf("bne instruction ");
      offset = ir & 0377;
      printf("with offset 0%03o\n", offset);
      offset = offset << 24;
      offset = offset >> 24;
      if(!z){
        reg[7] = (reg[7] + (offset << 1) ) & 0177777;
        numBranchesTaken++;
      }
      numBranches++;
      instructionNum++;

      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
    } else if((ir >> 8) == 003) {
      printf("beq instruction ");
      offset = ir & 0377;
      printf("with offset 0%03o\n", offset);
      offset = offset << 24;
      offset = offset >> 24;


      if(z == 1){
        reg[7] = (reg[7] + (offset << 1)) & 0177777;
        numBranchesTaken++;
      }
      numBranches++;
      instructionNum++;
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
    } else if((ir >> 6) == 0062 ){
      printf("asr instruction ");
      update_phrase_addr_values(&dst);
      c = dst.value & 1;
      final = dst.value << 16;
      final = final >> 16;
      final = final >> 1;
      final = final & 0177777;
      sign = final & 0100000;
      if(sign){
        n = 1;
      }else{
        n = 0;
      }
      if( final == 0){
      z = 1;
      }else{
      z = 0;
      }
      z = 0;
      if(n ^ c){
        v = 1;
      }else{
        v = 0;
      }
      printf("dm %o dr %o\n", dst.mode, dst.reg);
      printf("  dst.value = 0%06o\n", dst.value);
      printf("  result    = 0%06o\n", final);
      printf("  nzvc bits = 4'b%o%o%o%o\n", n, z, v, c);
      opUpdate(&dst,final);
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      instructionNum++;
    }
    else if((ir >> 6)== 0063)
    {
      printf("asl instruction ");
      update_phrase_addr_values( &dst );
      final = dst.value << 1;
      c = (final & 0200000) >> 16;
      final = final & 0177777;

      sign = final & 0100000;
      if(sign){
        n = 1;
      }else{
        n = 0;
      }
      if(final == 0){
      z = 1;
      }else{
      z = 0;
      }
      v = 0;
      if(n ^ c){
        v = 1;
      }
      printf("dm %o dr %o\n", dst.mode, dst.reg);
      printf("  dst.value = 0%06o\n", dst.value);
      printf("  result    = 0%06o\n", final);
      printf("  nzvc bits = 4'b%o%o%o%o\n", n, z, v, c);
      opUpdate( &dst, final );
      printf("  R0:%07o  R2:%07o  R4:%07o  R6:%07o\n", reg[0], reg[2], reg[4], reg[6]);
      printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);
      instructionNum++;
    } else {
      readData++;
      numFetches++;
    }
  }
  printf("\nexecution statistics (in decimal):\n");
  printf("  instructions executed     = %d\n", instructionNum);
  printf("  instruction words fetched = %d\n", numFetches);
  printf("  data words read           = %d\n", readData);
  printf("  data words written        = %d\n", writeData);
  printf("  branches executed         = %d\n", numBranches);
  printf("  branches taken            = %d", numBranchesTaken);
  double percent = ((double) numBranchesTaken/(double)numBranches) * 100;
  if(numBranches==0){
    printf("  branches taken          = %d\n",numBranches);
  }else{
    printf("(%.1f%%)\n",100.0*((double)numBranchesTaken)/(double)numBranches);
  }
  printf("\nfirst 20 words of memory after execution halts:\n");
  int num;
  for (int i = 0; i < 20; i++) {
    printf("  %05o: %06o\n", num, mem[i]);
    num+=2;
  }
  return 0;
}
void update_phrase_addr_values( addr_phrase_t *phrase ){
  assert((phrase->mode >= 0) && (phrase->mode <= 7));
  assert((phrase->reg  >= 0) && (phrase->reg  <= 7));
  switch(phrase->mode) {
    case 0:
      phrase->value = reg[phrase->reg];
      assert(phrase->value < 0200000);
      phrase->addr = 0;
      break;
    case 1:
      phrase->addr = reg[phrase->reg];
      assert( phrase->value < 0200000 );
      phrase->value = mem[phrase->addr >> 1];
      numFetches++;
      readData++;
      break;
    case 2:
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000 );
      phrase->value = mem[phrase->addr >> 1];
      assert(phrase->value < 0200000);
      reg[phrase->reg] = (reg[phrase->reg]+ 2) & 0177777;
      numFetches++;
      break;
    case 3:
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      phrase->value = mem[phrase->addr >> 1];
      assert(phrase->addr < 0200000);
      reg[ phrase->reg ] = (reg[phrase->reg] + 2 ) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      phrase->value = mem[phrase->addr];
      assert(phrase->addr < 0200000 );
      if(phrase->reg ==7)
      { numFetches++;
        readData++;
      }
      else{
        readData += 2;
      }
      break;
    case 4:
      reg[phrase->reg ] = (reg[phrase->reg] - 2) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      phrase->value = mem[phrase->addr >> 1];
      assert(phrase->value < 0200000);
      readData++;
      break;
    case 5:
      reg[phrase->reg] = (reg[phrase->reg]- 2) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      if (phrase->reg == 7)
      {
        numFetches++;
        readData++;
      }
      else{
        readData += 2;
      }
      break;
    case 6:
      phrase->addr = reg[phrase->reg];
      assert(phrase->value < 0200000);
      phrase->value = mem[ phrase->addr];
      assert(phrase->addr < 0200000);
      phrase->addr = phrase->value + ((reg[phrase->reg] + 2) & 0177777);
      assert(phrase->addr < 0200000);
      reg[7] += 2;
      readData += 2;
    break;
    case 7:
      phrase->addr = reg[phrase->reg];
      assert(phrase->value < 0200000);
      phrase->value = mem[phrase->addr];
      assert(phrase->addr<0200000);
      phrase->addr = phrase->value +((reg[phrase->reg]+2) & 0177777);
      assert(phrase->addr < 0200000);
      reg[7] += 2;
      readData += 2;
      break;
    default:
      printf("unimplemented address mode %o\n", phrase->mode);
  }
}
void opUpdate( addr_phrase_t *phrase, int final ) {
    update_phrase_addr_values(phrase);
  if ( phrase->mode == 0 ) {
    reg[phrase->reg] = final;
  } else {
    mem[phrase->addr] = final;
  }
}
