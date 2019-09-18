#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <assert.h>

static struct
{
  unsigned int num_page_fault;
  unsigned int num_page_try;
  unsigned int num_tlb_miss;
  unsigned int num_tlb_try;
} perf_stat = {
    .num_page_fault = 0,
    .num_page_try = 0,
    .num_tlb_miss = 0,
    .num_tlb_try = 0};

struct tlb_entry
{
  uint8_t page_num;
  uint8_t frame_num;
  int is_valid; // 1 is valid or not.
};

struct page_entry
{
  uint16_t frame_num;
  int is_valid; // 1 is valid or not.
};

uint8_t lookup_tlb(uint8_t page_num);
uint8_t lookup_page_table(uint8_t page_num);
uint8_t lookup_phy_mem(uint32_t phy_addr);

void page_fault_handler(uint8_t page_num);
uint32_t to_phy_addr(uint32_t virt_addr);

struct tlb_entry tlb[16];
int tlb_fifo_idx = 0;
struct page_entry page_table[256];
uint8_t phy_mem[256 * 256] = {
    0,
};

int main(int argc, char **argv)
{
  // Clean tlb and page table.
  for (int it = 0; it < 16; ++it)
  {
    tlb[it].is_valid = 0;
  }
  for (int it = 0; it < 256; ++it)
  {
    page_table[it].is_valid = 0;
  }

  uint32_t virt_addr;
  while (scanf("%u", &virt_addr) != EOF)
  {

    uint32_t phy_addr = to_phy_addr(virt_addr);

    fprintf(stderr, "%d\n", lookup_phy_mem(phy_addr));
  }
  /*
  for (int i = 38 ; i < 40; i++)
  {
    for (int j = 0; j < 256; j++)
    {
      printf("%d: 0x%x\n", i*256+j, phy_mem[i*256 + j]);
    }
    printf("\n");
  }
  */

  printf("pf: %lf\ntlb: %lf\n",
         (double)perf_stat.num_page_fault / perf_stat.num_page_try,
         (double)perf_stat.num_tlb_miss / perf_stat.num_tlb_try);

  return 0;
}

uint8_t lookup_tlb(uint8_t page_num)
{
  perf_stat.num_tlb_try++;

  for (struct tlb_entry *it = tlb; it < tlb + 16; it++)
  {
    if (it->is_valid && it->page_num == page_num)
    {
      return it->frame_num;
    }
  }

  perf_stat.num_tlb_miss++;

  uint8_t frame_num = lookup_page_table(page_num);
  //printf("\t\t\t\t\tframe_num: 0x%x\n", frame_num);
  struct tlb_entry *it = tlb + tlb_fifo_idx;
  tlb_fifo_idx = ++tlb_fifo_idx % 16;
  //printf("tlb index: %d\n", tlb_fifo_idx);
  it->page_num = page_num;
  it->frame_num = frame_num;
  it->is_valid = 1;

  return it->frame_num;
}

uint8_t lookup_page_table(uint8_t page_num)
{
  //printf("page_num: 0x%x\n", page_num);
  if (!page_table[page_num].is_valid)
  {
    page_fault_handler(page_num);
    perf_stat.num_page_fault++;
  }
  
  assert(page_table[page_num].is_valid);

  perf_stat.num_page_try++;

  return page_table[page_num].frame_num;
}

void page_fault_handler(uint8_t page_num)
{
  FILE *fp = fopen("./input/BACKINGSTORE.bin", "r");  // ./input/BACKINGSTORE.bin 으로 수정 필요
  

  if(fp == NULL) {
    perror("File Open Error\n");
    exit(1);
  }
  
  // TODO: Fill this!
  fseek(fp, page_num*256L, SEEK_SET);
  
  page_table[page_num].is_valid = 1;
  
  for (int i = 0; i < 256; i++)
  {
    phy_mem[page_num*256 + i] = fgetc(fp);
  }

  page_table[page_num].frame_num = page_num;

  fclose(fp);
}

uint32_t to_phy_addr(uint32_t virt_addr)
{
  uint8_t _page_num = (virt_addr & 0xFF00) >> 8;
  uint8_t _offset = virt_addr & 0xFF;
  //printf("\tvirt_addr: 0x%x, page num: 0x%x, offset: 0x%x\n",virt_addr , _page_num, _offset);
  uint8_t _frame_num = lookup_tlb(_page_num);
  uint32_t _phy_addr = (_frame_num << 8) | _offset;
  //printf("0x%x 0x%x\n", _page_num, _frame_num);
  //printf("0x%x 0x%x 0x%x\n", _page_num, _offset, virt_addr);
  return _phy_addr; // TODO: Make it work!
}

uint8_t lookup_phy_mem(uint32_t phy_addr)
{
  uint8_t _frame_num = (phy_addr & 0xFF00) >> 8;
  uint8_t _offset = phy_addr & 0xFF;
  //printf("0x%x 0x%x 0x%x\n", _frame_num, _offset, phy_addr);
  //printf("%x :"_frame_num*256 + _offset);
  return phy_mem[_frame_num*256 + _offset]; // TODO: Make it work!
}
