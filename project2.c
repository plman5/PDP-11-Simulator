#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define MEM_SIZE_IN_WORDS 32*1024
#define LINES_PER_BANK 16
typedef struct ap{
  int mode;
  int reg;
  int addr;
  int value;
} addr_phrase_t;
unsigned int
  plru_state[LINES_PER_BANK],
  valid[4][LINES_PER_BANK],
  dirty[4][LINES_PER_BANK],
  tag[4][LINES_PER_BANK],
  plru_bank[8] = { 0, 0, 1, 1, 2, 3, 2, 3 },
  next_state[32]/* table for next state based on state and bank ref */
                 /* index by 5-bit (4*state)+bank [=(state<<2)|bank] */
                                    /*  bank ref  */
                                    /* 0  1  2  3 */
                 /*         0 */  = {  6, 4, 1, 0,
                 /*         1 */       7, 5, 1, 0,
                 /*         2 */       6, 4, 3, 2,
                 /* current 3 */       7, 5, 3, 2,
                 /*  state  4 */       6, 4, 1, 0,
                 /*         5 */       7, 5, 1, 0,
                 /*         6 */       6, 4, 3, 2,
                 /*         7 */       7, 5, 3, 2};
void putOperand(addr_phrase_t *phrase, int result);
void update_phrase_addr_values( addr_phrase_t *phrase );
void opUpdate( addr_phrase_t *phrase, int final );
void cache_init(void);
void cache_access(uint8_t address, uint8_t type);
void cache_stats(void);
int numBranches= 0;
int numBranchesTaken=0;
int instructionNum =0;
int numFetches= 0;
int readData =0;
int writeData= 0;
int mem[MEM_SIZE_IN_WORDS] = {0};
int reg[8] = {0};
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
unsigned int
    cache_reads,
    cache_writes,
    hits,
    misses,
    write_backs;
int main(int argc, char* argv[]) {

  int i=0;
  int input;
  FILE* inFile = fopen("branch.in.txt","r");
  //int input = scanf("%o",&ir);
  printf("reading words in octal from stdin:\n");
  while(scanf("%o",&input) != EOF){
    mem[i] = input;
   // cache_access(i,1); 
    printf("  0%06o\n",mem[i]);
    i+=1;
  }
  cache_init();
 //  cache_access(reg[7],0);
  while(!halt){
    //cache_access(reg[7],0);
    ir = mem[reg[7]>>1];

    assert( ir < 0200000 );
    cache_access(reg[7],0);
    reg[7] = (reg[7]+ 2) & 0177777;
    src.mode = (ir >> 9) & 07;
    src.reg = (ir >> 6) & 07;
    dst.mode = (ir >> 3) & 07;
    dst.reg = (ir) & 07;

    numFetches++;
  if (ir == 0){

      halt = 1;
      instructionNum++;
    }else if((ir >> 12)== 01){
    //  printf("mov instruction ");
    
      int tempReg = reg[dst.reg];
      if (dst.mode == 2){
        update_phrase_addr_values(&src);
        int tempMem = tempReg >> 1;
        mem[tempMem] = src.value;
        //cache_access(dst.addr,1);
        // put operand dest here
        putOperand(&dst,src.value);
       // cache_access(dst.addr,1);
       //opUpdate(&dst,src.value);
        //update_phrase_addr_values(&dst);
        numFetches-=1;
      } else {
        update_phrase_addr_values(&src);
        update_phrase_addr_values(&dst);
        dst.value = src.value;
        final = dst.value;
        opUpdate(&dst,final);
       // putOperand(&dst,src.value);
        // put result that clones get operand except last thing in each case is a write instead of a read
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
     // c = 0;

      if (dst.mode == 2) {
      //  printf("  value 0%06o is written to 0%06o\n", src.value, tempReg);
      //cache_access(dst.addr,1);
       // putOperand(&dst,final);
        writeData++;
      }
      instructionNum++;
    } else if((ir >> 12)==02){
    //  printf("cmp instruction ");
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
    //  printf("  R1:%07o  R3:%07o  R5:%07o  R7:%07o\n", reg[1], reg[3], reg[5], reg[7]);;
      instructionNum++;


      }else if((ir >> 12)== 06){
    //  printf("add instruction ");
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
      opUpdate(&dst,final);
      //putOperand(&dst,final);
      instructionNum++;
    } else if((ir >> 12 ==016)){
  //    printf("sub instruction ");
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


      opUpdate(&dst,final);

      instructionNum++;
    }else if((ir >> 9)==077){
  //    printf("sob instruction ");
      offset = ir & 037;
      update_phrase_addr_values(&dst);
      dst.value -= 1;
      reg[dst.reg] = dst.value;
      final = dst.value;
  //    printf("reg %d with offset 0%02o\n", dst.reg, offset);
      offset = offset << 24;
      offset = offset >> 24;
      if (final!= 0){
        reg[7] = (reg[7] - (offset << 1)) & 0177777;
        numBranchesTaken++;
      }
      numBranches++;
      instructionNum++;

      }else if((ir >> 8) == 001){
  //    printf("br instruction ");
      offset = ir & 0377;
  //    printf("with offset 0%03o\n", offset);
      offset = offset << 24;
      offset = offset >> 24;
      reg[7] = (reg[7] + (offset<< 1) ) &0177777;
      numBranchesTaken++;
      numBranches++;
      instructionNum++;

      } else if((ir >> 8)== 002){
    //  printf("bne instruction ");
      offset = ir & 0377;
    //  printf("with offset 0%03o\n", offset);
      offset = offset << 24;
      offset = offset >> 24;
      if(!z){
        reg[7] = (reg[7] + (offset << 1) ) & 0177777;
        numBranchesTaken++;
      }
      numBranches++;
      instructionNum++;
    } else if((ir >> 8) == 003) {
    //  printf("beq instruction ");
      offset = ir & 0377;
  //    printf("with offset 0%03o\n", offset);
      offset = offset << 24;
      offset = offset >> 24;


      if(z == 1){
        reg[7] = (reg[7] + (offset << 1)) & 0177777;
        numBranchesTaken++;
      }
      numBranches++;
      instructionNum++;

    } else if((ir >> 6) == 0062 ){
    //  printf("asr instruction ");
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
      instructionNum++;
    }
    else if((ir >> 6)== 0063)
    {
    //  printf("asl instruction ");
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
  cache_stats();
  return 0;
}
void putOperand(addr_phrase_t *phrase, int final){
  assert((phrase->mode >= 0) && (phrase->mode <= 7));
  assert((phrase->reg  >= 0) && (phrase->reg  <= 7));
  switch(phrase->mode) {
    case 0:
      phrase->value = reg[phrase->reg];
      assert(phrase->value < 0200000);
      phrase->addr = 0;
      break;
    case 1:
     // cache_access(phrase->addr,1);

      phrase->addr = reg[phrase->reg];
      assert( phrase->value < 0200000 );
      mem[phrase->addr] = final;
      cache_access(phrase->addr,1);
      numFetches++;
      writeData++;
      break;
    case 2:
      //cache_access(phrase->addr,1);
      //cache_access(reg[7],1);
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000 );
      mem[phrase->addr] = final;
      assert(phrase->value < 0200000);
      reg[phrase->reg] = (reg[phrase->reg]+ 2) & 0177777;
      cache_access(reg[phrase->reg],1);
      numFetches++;
      break;
    case 3:
     // cache_access(phrase->addr,1);
   //  cache_access(reg[7],1);
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      mem[phrase->addr] = final;
      assert(phrase->addr < 0200000);
      reg[ phrase->reg ] = (reg[phrase->reg] + 2 ) & 0177777;
     //cache_access(reg[7],1);
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
       mem[phrase->addr] = final;

      assert(phrase->addr < 0200000 );
      if(phrase->reg ==7)
      { cache_access(reg[phrase->reg],1);
        numFetches++;
        writeData++;
      }
      else{
        cache_access(reg[phrase->reg],1);
        writeData += 2;
      }
      break;
    case 4:
      //cache_access(phrase->addr,1);

      reg[phrase->reg ] = (reg[phrase->reg] - 2) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
       mem[phrase->addr] = final;

      assert(phrase->value < 0200000);
      cache_access(reg[phrase->reg],1);
      writeData++;
      break;
    case 5:
      reg[phrase->reg] = (reg[phrase->reg]- 2) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      if (phrase->reg == 7)
      {
        cache_access(reg[phrase->reg],1);
        numFetches++;
        writeData++;
      }
      else{
        cache_access(reg[phrase->reg],1);
        writeData += 2;
      }
      break;
    case 6:
      //cache_access(phrase->addr,1);
      //cache_access(reg[7],1);
      phrase->addr = reg[phrase->reg];
      assert(phrase->value < 0200000);
      mem[ phrase->addr] = final;
      assert(phrase->addr < 0200000);
      phrase->addr = phrase->value + ((reg[phrase->reg] + 2) & 0177777);
      assert(phrase->addr < 0200000);
      cache_access(phrase->addr,1);
      reg[7] += 2;
      writeData += 2;
    break;
    case 7:
      //cache_access(phrase->addr,1);
      //cache_access(reg[7],1);
      phrase->addr = reg[phrase->reg];
      assert(phrase->value < 0200000);
      mem[phrase->addr] = final;
      assert(phrase->addr<0200000);
      phrase->addr = phrase->value +((reg[phrase->reg]+2) & 0177777);
      assert(phrase->addr < 0200000);
      cache_access(phrase->addr,1);
      reg[7] +=2;
      writeData += 2;
      break;
    default:
      printf("unimplemented address mode %o\n", phrase->mode);
  }
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
      //cache_access(phrase->addr,0);
    //  cache_access(reg[7],0);
      phrase->addr = reg[phrase->reg];
      assert( phrase->value < 0200000 );
      phrase->value = mem[phrase->addr >> 1];
      cache_access(phrase->addr,0);
      numFetches++;
      readData++;
      break;
    case 2:
      //cache_access(phrase->addr,0);
      //cache_access(reg[7],0);
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000 );
      phrase->value = mem[phrase->addr >> 1];

      assert(phrase->value < 0200000);
      reg[phrase->reg] = (reg[phrase->reg]+ 2) & 0177777;
      cache_access(reg[phrase->reg],0);
      numFetches++;
      break;
    case 3:
    //  cache_access(phrase->addr,0);
     // cache_access(reg[7],0);
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      phrase->value = mem[phrase->addr >> 1];

      assert(phrase->addr < 0200000);
      reg[ phrase->reg ] = (reg[phrase->reg] + 2 ) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      phrase->value = mem[phrase->addr];
      //cache_access(phrase->addr,0);
      assert(phrase->addr < 0200000 );
      if(phrase->reg ==7)
      {  cache_access(reg[phrase->reg],0);
        numFetches++;
        readData++;
      }
      else{
         cache_access(reg[phrase->reg],0);
        readData += 2;
      }
      break;
    case 4:
    //  cache_access(phrase->addr,0);
     // cache_access(reg[7],0);
      reg[phrase->reg ] = (reg[phrase->reg] - 2) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      phrase->value = mem[phrase->addr >> 1];

      assert(phrase->value < 0200000);
       cache_access(reg[phrase->reg],0);
      readData++;
      break;
    case 5:
      reg[phrase->reg] = (reg[phrase->reg]- 2) & 0177777;
      phrase->addr = reg[phrase->reg];
      assert(phrase->addr < 0200000);
      if (phrase->reg == 7)
      {
         cache_access(reg[phrase->reg],0);
        numFetches++;
        readData++;
      }
      else{
         cache_access(reg[phrase->reg],0);
        readData +=2;
      }
      break;
    case 6:
     // cache_access(phrase->addr,0);
      //cache_access(reg[7],0);
      phrase->addr = reg[phrase->reg];
      assert(phrase->value < 0200000);
      phrase->value = mem[ phrase->addr];

      assert(phrase->addr < 0200000);
      phrase->addr = phrase->value + ((reg[phrase->reg] + 2) & 0177777);
      assert(phrase->addr < 0200000);
       cache_access(phrase->addr,0);
      reg[7] += 2;
      readData += 2;
    break;
    case 7:
     // cache_access(phrase->addr,0);
     // cache_access(reg[7],0);
      phrase->addr = reg[phrase->reg];
      assert(phrase->value < 0200000);
      phrase->value = mem[phrase->addr];

      assert(phrase->addr<0200000);
      phrase->addr = phrase->value +((reg[phrase->reg]+2) & 0177777);
      assert(phrase->addr < 0200000);
      cache_access(phrase->addr,0);
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
    //cache_access(reg[7],1);
    mem[phrase->addr] = final;

  }
}
void cache_init( void ){
  int i;
  for( i=0; i<LINES_PER_BANK; i++ ){
    plru_state[i] = 0;
    valid[0][i] = dirty[0][i] = tag[0][i] = 0;
    valid[1][i] = dirty[1][i] = tag[1][i] = 0;
    valid[2][i] = dirty[2][i] = tag[2][i] = 0;
    valid[3][i] = dirty[3][i] = tag[3][i] = 0;
  }
  cache_reads = cache_writes = hits = misses = write_backs = 0;
}
void cache_stats( void ){
  printf( "cache statistics (in decimal):\n" );
  printf( "  cache reads       = %d\n", cache_reads );
  printf( "  cache writes      = %d\n", cache_writes );
  printf( "  cache hits        = %d\n", hits );
  printf( "  cache misses      = %d\n", misses );
  printf( "  cache write backs = %d\n", write_backs );
}
void cache_access( uint8_t address, uint8_t type ){
  unsigned int
    addr_tag,    /* tag bits of address     */
    addr_index,  /* index bits of address   */
    bank;        /* bank that hit, or bank chosen for replacement */
  if( type == 0 ){
    printf("cache read at %o\n",address);
    cache_reads++;
  }else{
    printf("cache write at %o\n",address);
    cache_writes++;
  }
  addr_index = (address >> 3) & 0xf;
  addr_tag = address >> 7;
  /* check bank 0 hit */
  if( valid[0][addr_index] && (addr_tag==tag[0][addr_index]) ){
    hits++;
    bank = 0;
  /* check bank 1 hit */
  }else if( valid[1][addr_index] && (addr_tag==tag[1][addr_index]) ){
    hits++;
    bank = 1;
  /* check bank 2 hit */
  }else if( valid[2][addr_index] && (addr_tag==tag[2][addr_index]) ){
    hits++;
    bank = 2;
  /* check bank 3 hit */
  }else if( valid[3][addr_index] && (addr_tag==tag[3][addr_index]) ){
    hits++;
    bank = 3;
  /* miss - choose replacement bank */
  }else{
    misses++;
         if( !valid[0][addr_index] ) bank = 0;
    else if( !valid[1][addr_index] ) bank = 1;
    else if( !valid[2][addr_index] ) bank = 2;
    else if( !valid[3][addr_index] ) bank = 3;
    else bank = plru_bank[ plru_state[addr_index] ];
    if( valid[bank][addr_index] && dirty[bank][addr_index] ){
      write_backs++;
    }
    valid[bank][addr_index] = 1;
    dirty[bank][addr_index] = 0;
    tag[bank][addr_index] = addr_tag;
  }
  /* update replacement state for this set (i.e., index value) */
  plru_state[addr_index] = next_state[ (plru_state[addr_index]<<2) | bank ];
  /* update dirty bit on a write */
  if( type == 1 ) dirty[bank][addr_index] = 1;
}
