#include <iostream>
#include <string>
#include <vector>
#include <iomanip> 
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include "parse.h"
using namespace std;

class Cache
{
  public:
    uint32_t size, assoc, block_size, address;uint32_t sets, block_offset, setsbits, tagbits;
    unordered_map<uint32_t,unordered_map<uint32_t, pair<bool, char>>> cachememory; 
    unordered_map<uint32_t,unordered_map<uint32_t,int>> LRU_counter;
    bool debug= false;
    /*
    1st int is set so set match happens first and 2nd int should be tag after set match tag match should happen, 
    then check for valid and then get data and take care of the dirty bit
    */
    Cache(uint32_t size, uint32_t assoc, uint32_t block_size)
    {
        this->size = size;
        this->assoc = assoc;
        this->block_size = block_size;
        sets = this->size / (this->assoc * this->block_size);  //Number of sets

        block_offset = (int)log2(1.0 * this->block_size);  // bits for block offset
        setsbits = (int)log2(1.0 * (sets));  // bits for set index
        tagbits = 32 - block_offset - setsbits;
    }
    bool checkhit(uint32_t address,char c) //This function doesn't check dirty bit this has to be done in a different function
    {
      //cout<<" Received address: "<< address<<endl;
      this->address = address;
      uint32_t block_offset_value = address & ((1 << block_offset) - 1);
      uint32_t set_value = (address >> block_offset) & ((1 << setsbits) - 1);
      uint32_t tag_value = address >> (block_offset + setsbits);
      //cout<<" Set: "<<set_value<<" "<<"tag : "<<tag_value<<endl;
      //cout<<set_value<<endl;
      if(cachememory.find(set_value) != cachememory.end())
      {
        if(cachememory[set_value].find(tag_value) != cachememory[set_value].end()) //The set is present
        {
          //Marking the cache dirty
        //cout<<endl;
          cachememory[set_value][tag_value].second = ((c == 'r' || c==' ' || c=='N') && cachememory[set_value][tag_value].second != 'D')?' ':'D';
          if(LRU_counter[set_value][tag_value] == 0) //if this tag is already the most recently used 
          {
            if(debug)
            {cout<<"Hit with empty"<<endl;
            for(auto &it : cachememory[set_value])
            {

              cout<<"Set :"<<dec<<set_value<<" Tag: "<<it.first<<" ";
            }
            cout<<endl;}
          return true;}
          else
          {
            //cout<<"Hit for this tag: "<<tag_value<<endl;
            int val = LRU_counter[set_value][tag_value];
            LRU_counter[set_value][tag_value] = 0;
            for(auto &it : LRU_counter[set_value])
            {
              uint32_t tag = it.first;
              //cout<<" Value: "<<val<<endl;
              if(((int)tag != (int)tag_value) && (LRU_counter[set_value][tag] < val))
              {LRU_counter[set_value][tag]++;
              }
            }
            if(debug)
            {
              cout<<"Hit"<<endl;
              for(auto &it : cachememory[set_value])
            {
              cout<<"Set :"<<dec<<set_value<<" Tag: "<<it.first<<" "<<"Count: "<<LRU_counter[set_value][it.first]<<" ";
            }
            cout<<endl;
            }
          }
        return true;
          
        }
        else
        {
            return false;
        }
      }
        else
        {
          //cout<<"Something wrong with your set values"<<endl;
          return false;
        }
        return false;
    }
    pair<int,char> misshandle(uint32_t address,char c) //put character
    {
      //Returning type is char as I wanted to see if they are dirty and we should do a write miss
      uint32_t block_offset_value = address & ((1 << block_offset) - 1);
      uint32_t set_value = (address >> block_offset) & ((1 << setsbits) - 1);

      uint32_t tag_value = address >> (block_offset + setsbits);
      if(cachememory.find(set_value) == cachememory.end()) // the set isn't accessed at all still
      {
        //cout<<"Set being accessed for the first time"<< endl;
        cachememory[set_value][tag_value].first = true;
        cachememory[set_value][tag_value].second = (c=='r' || c==' ' || c=='N')?' ':'D';
        LRU_counter[set_value][tag_value] = 0;
        if(debug)
            {
              cout<<"Hit"<<endl;
              for(auto &it : cachememory[set_value])
            {
              cout<<"Set :"<<dec<<set_value<<" Tag: "<<it.first<<" "<<"Count: "<<LRU_counter[set_value][it.first]<<" ";
            }
            cout<<endl;
            }
        return make_pair(0,'N');  
      }
      else if(cachememory[set_value].size()< (int)assoc) // set is accessed
      {
        int a = cachememory[set_value].size();
        //cout<<"Set value Size:"<<cachememory[set_value].size()<<endl;
        //cout<<"Associativity: "<<assoc<<endl;
        //print_cache();
        cachememory[set_value][tag_value].first = true;
        cachememory[set_value][tag_value].second = (c=='r' || c==' ' || c=='N')?' ':'D';
        LRU_counter[set_value][tag_value] = 0;

        for(auto &i : LRU_counter[set_value])
        {
          uint32_t tag = i.first;
          uint32_t count = i.second;
          if(tag!= tag_value)
          {
            //if(i.second > assoc)
            //cout<<"1st else if"<<endl;
            i.second++;
            //if(i.second >= (int)assoc)
            //cout<<"Set size might have increased"<<endl;
          }
        }
        if(debug)
            {
              cout<<"Hit"<<endl;
              for(auto &it : cachememory[set_value])
            {
              cout<<"Set :"<<dec<<set_value<<" Tag: "<<it.first<<" "<<"Count: "<<LRU_counter[set_value][it.first]<<" ";
            }
            cout<<endl;
            }
        //cout<<"Did I come here"<<endl;
        return make_pair(0,'N');
      }
      else if(cachememory[set_value].size() == assoc) // the line is filled completely, no empty spaces
      {
        // I have to replace the number whose LRU_counter value = assoc-1
        char senddirty; //variable required to send whether the line was dirty or not
        pair<int,char>  s;
        bool flag = false;
        int l=0;
        /*Trying for VC remove it later*/
        LRU_counter[set_value][tag_value] = 0;
        cachememory[set_value][tag_value].first = true;
        cachememory[set_value][tag_value].second = (c=='r' || c==' ' || c=='N')?' ':'D';
        uint32_t evicttag = 0;
        for(auto& i : LRU_counter[set_value])
        {
          uint32_t tag = i.first;
          int count = i.second;
          if(count == (int)assoc -1 && tag!= tag_value)
          {
            evicttag = tag;
            break;
          }
        }
        LRU_counter[set_value].erase(evicttag);
        uint32_t send = evicttag<<(setsbits+block_offset) | set_value<<(block_offset);
        senddirty = cachememory[set_value][evicttag].second;
        cachememory[set_value].erase(evicttag);
        s = make_pair(send,senddirty);
        for(auto &i : LRU_counter[set_value])
        {
          
          uint32_t tag = i.first;
          int count = i.second;
         if(tag != tag_value)
         if(count!=((int)assoc-1))
         i.second = i.second + 1;
        }
        if(debug)
            {
              cout<<"Hit"<<endl;
              for(auto &it : cachememory[set_value])
            {
              cout<<"Set :"<<dec<<set_value<<" Tag: "<<it.first<<" "<<"Count: "<<LRU_counter[set_value][it.first]<<" ";
            }
            cout<<endl;
            }
        return s;
      }
      return make_pair(0,'N');      
    }
    void print_cache() {
    
    // Get the set values (keys from outer map) and store them in a vector to sort
    vector<uint32_t> sets;
    for (const auto& set_entry : cachememory) {
        sets.push_back(set_entry.first);
    }
    
    // Sort the set values in ascending order
    sort(sets.begin(), sets.end());
    
    // Iterate over the sorted set values
    for (uint32_t set_value : sets) {
        // Retrieve the tag map for this set
        const auto& tag_map = cachememory.at(set_value);
        const auto& lru_map = LRU_counter.at(set_value);
        
        // Create a vector of tag and LRU pairs to sort based on LRU counter values
        vector<pair<uint32_t, int>> tags_lru;
        for (const auto& tag_entry : lru_map) {
            tags_lru.push_back(make_pair(tag_entry.first, tag_entry.second));
        }
        
        // Sort tags based on LRU counter (smallest first, least recently used)
        sort(tags_lru.begin(), tags_lru.end(), [](const pair<uint32_t, int>& a, const pair<uint32_t, int>& b) {
            return a.second < b.second;
        });
        
        // Print the set value
        cout << "\tset\t" << std::dec<<set_value << ":\t";
        
        // Print the tags in LRU order along with the dirty bit
        for (const auto& tag_lru : tags_lru) {
            uint32_t tag = tag_lru.first;
            char dirty_bit = tag_map.at(tag).second; // Access dirty bit from cachememory
            
            cout << hex << tag << " " << dirty_bit << "\t"; // Print tag and dirty bit in hex format
        }
        cout << endl;
    }
  }
};
void printLRUcounter(Cache* L)
{
  for(auto &it: L->LRU_counter)
  {
    uint32_t set_value = it.first;
    unordered_map<uint32_t,int>& tag_counts = it.second;
    for(auto &i : tag_counts)
    {
    cout<<"Tag: "<<std::hex<<i.first<<" Count:"<<i.second;
    }
    cout<<endl;
  }
}
void printstats(bool vc, bool l2, Cache *L1, Cache *L2, Cache *VC, uint32_t l1size,uint32_t l1assoc, uint32_t blocksize, uint32_t l2size,uint32_t l2assoc, uint32_t vcblocks, string file_name,int l1reads, int l1readmisses, int l1writes, int l1writemisses, int swaprequests,int l1vcwriteback, int swaps, int l2reads, int l2readmisses, int l2writes, int l2writemisses, int l2writebacks);
void L1VC(Cache* L1, Cache* VC,uint32_t l1assoc, string s)
{
  ifstream infile(s);
  if(!infile)
  {
    cerr<<"Error Opening File"<<endl;
    return;
  }
  string line;
  vector<pair<uint32_t,char>> trace;
  int a=0;
  int l1reads = 0;
  int l1writes = 0;
  int l1readmiss = 0;
  int l1writemiss = 0;
  int swaprequests =0;
  int swaps =0;
  int writebacks = 0;
  while(getline(infile, line))
  {
    std::stringstream ss(line);
    char operation;
    std::string hex_str;
    uint32_t hex_value;
    ss >> operation >> hex_str;

    // Convert the hexadecimal string to a uint32_t integer
    std::stringstream hex_stream;
    hex_stream << std::hex << hex_str;
    hex_stream >> hex_value;
    //std::cout<<hex_value<<endl;
    trace.push_back(make_pair(hex_value,operation));
    //cout<<std::hex<<trace[a].first<<endl;
    a++;
  }
  for(int i=0;i<trace.size();i++)
  {
    /*
    We have 3 scenarios
    L1 hit - Just update the LRU counter
    L1 miss VC hit - Swap LRU of L1 with the value of VC and update the LRU but don't evict the last one
    L1 miss VC miss - Put LRU of L1 to VC and enter VC if and only if L1 is full
    */
    bool fl1 = L1->checkhit(trace[i].first, trace[i].second);
    //I have to check whether the set is full or not first
    //If full then only enter victim cache.
    uint32_t block_offset_value = trace[i].first & ((1 << L1->block_offset) - 1);
    uint32_t set_value = (trace[i].first >> L1->block_offset) & ((1 << L1->setsbits) - 1);
    uint32_t tag_value = trace[i].first >> (L1->block_offset + L1->setsbits);
    if(trace[i].second == 'r')
    l1reads++;
    else
    l1writes++;
    if(!fl1)
    {
      //Miss in L1, have to check miss in VC
      if(trace[i].second == 'w')
      //write miss in L1
      l1writemiss++;
      else
      l1readmiss++;

      if(L1->cachememory[set_value].size() == l1assoc)
      { //VC miss
      //cout<<"L1 miss"<<endl;
      swaprequests++;
      if(!(VC->checkhit(trace[i].first, trace[i].second)))
      {
        //miss
        //cout<<"VC miss"<<endl;
        pair<uint32_t,char> p = L1->misshandle(trace[i].first,trace[i].second); // I have to remove the LRU block from Cache and put it in VC
        pair<uint32_t,char> l = VC->misshandle(p.first, p.second); // I have to remove the LRU of VC and remove it and add the evicted block from L1 cache.
        if(l.second == 'D')
        writebacks++;
      }
      else{
        //VC hit this is
        //swap values here so that LRU isn't removed
        //cout<<"LRU of victim before swap"<<endl;
        //printLRUcounter(VC);
        pair<uint32_t, char> p = L1->misshandle(trace[i].first, trace[i].second); // This replcaes the L1 cache line with the new line fron VC and then this replaces the VC line of that trace
        uint32_t block_offset_valuevc = p.first & ((1 << VC->block_offset) - 1);
        uint32_t set_valuevc = (p.first >> VC->block_offset) & ((1 << VC->setsbits) - 1);
        uint32_t tag_valuevc = p.first >> (VC->block_offset + VC->setsbits);
        uint32_t bovc = trace[i].first & ((1 << VC->block_offset) - 1);
        uint32_t svvc = (trace[i].first >> VC->block_offset) & ((1 << VC->setsbits) - 1);
        uint32_t tvvc = trace[i].first >> (VC->block_offset + VC->setsbits);
        if(VC->cachememory[svvc][tvvc].second == 'D')
        L1->cachememory[set_value][tag_value].second = 'D';
        //Updating the LRU of VC
        int val = VC->LRU_counter[svvc][tvvc];
        VC->cachememory[svvc].erase(tvvc);
        VC->LRU_counter[svvc].erase(tvvc);
        for(auto &it : VC->LRU_counter[set_valuevc])
        {
          int lru = it.second;
          uint32_t tag = it.first;
          /*I want to check if two values of same LRU are already there or not and then after updating I want to check*/

          if(lru < val)
          {
            VC->LRU_counter[set_valuevc][tag] = lru+1;
          }
        }
        VC->LRU_counter[set_valuevc][tag_valuevc] = 0;// LRU updation
        //Add the one evicted out from L1 to VC
        VC->cachememory[set_valuevc][tag_valuevc].first = true;
        VC->cachememory[set_valuevc][tag_valuevc].second = p.second;
        //Erase the content of the line which got swapped
        
        //Erase the LRU counter
        //counter has been erased above
        swaps++;
      }
      }
      else
      {
        //when cache line is not full
        pair<uint32_t, char> p = L1->misshandle(trace[i].first, trace[i].second);
      }
    }
  }
  printstats(true,false,L1,NULL,VC,L1->size,L1->assoc,L1->block_size,0,0,VC->assoc,s,l1reads,l1readmiss,l1writes,l1writemiss,swaprequests,writebacks,swaps,0,0,0,0,0);
}
void L1(uint32_t size, uint32_t assoc, uint32_t block_size,string s)
{
  ifstream infile(s);
  if(!infile)
  {
    cerr<<"Error Opening File"<<endl;
    return ;
  }
  string line;
  vector<pair<uint32_t,char>> trace;
  int a=0;
  while(getline(infile, line))
  {
    std::stringstream ss(line);
    char operation;
    std::string hex_str;
    uint32_t hex_value;
    ss >> operation >> hex_str;

    // Convert the hexadecimal string to a uint32_t integer
    std::stringstream hex_stream;
    hex_stream << std::hex << hex_str;
    hex_stream >> hex_value;
    //cout<<hex_value;
    trace.push_back(make_pair(hex_value,operation));
    //cout<<std::hex<<trace[a].first<<endl;
    //cout<<hex<<trace[a].first<<endl;
    a++;
  }
  // uint32_t size = 1024;
  // uint32_t assoc = 1;
  // uint32_t block_size = 16;
  Cache L1(size,assoc,block_size);
  int c = 0;
  //cout<<"hi";
  int l1reads =0;
  int l1readmisses = 0;
  int l1writes = 0;
  int l1writemisses =0;
  int writebacks=0;
  for(int i=0;i<trace.size();i++)
  {
    bool f = L1.checkhit(trace[i].first,trace[i].second);
    l1reads = (trace[i].second == 'r')?l1reads+1:l1reads;
    l1writes = (trace[i].second == 'w')?l1writes+1:l1writes;
    uint32_t block_offset_value = L1.address & ((1 << L1.block_offset) - 1);
    uint32_t set_value = (L1.address >> L1.block_offset) & ((1 << L1.setsbits) - 1);
    uint32_t tag_value = L1.address >> (L1.block_offset + L1.setsbits);
    //cout<<"Address: "<<std::hex<<trace[i].first<<endl;
    //cout<<std::hex<<L1.address<<" block_offset_value: "<<std::hex<<block_offset_value<<" set_value: "<<std::hex<<set_value<<" tag_value: "<<std::hex<<tag_value<<endl;
    if(f == false)
    {
      l1readmisses = (trace[i].second =='r')?l1readmisses+1:l1readmisses;
      l1writemisses = (trace[i].second == 'w')?l1writemisses+1:l1writemisses;
      //cout<<"Miss"<<endl;
      c++;
      pair<int,char> x = L1.misshandle(trace[i].first,trace[i].second);
      //cout<<std::hex<<x.first<<endl;
      //cout<<x.second<<endl;
      if(x.second != 'N')
      {
        //cout<<"The cache line was full"<<endl;
        if(x.second == 'D')
        {
        writebacks++;
        }

      }
      //printcache(&L1);
      //printLRUcounter(&L1);
    }
  }
  printstats(false,false,&L1,NULL,NULL,size,assoc,block_size,0,0,0,s,l1reads,l1readmisses,l1writes,l1writemisses,0,writebacks,0,0,0,0,0,0);
}
void L1L2(uint32_t l1size, uint32_t l1assoc, uint32_t l1blocksize,uint32_t l2size, uint32_t l2assoc, uint32_t l2blocksize,string s)
{
  Cache L1(l1size,l1assoc, l1blocksize);
  Cache L2(l2size, l2assoc, l2blocksize);
  ifstream infile(s);
  if(!infile)
  {
    cerr<<"Error Opening File"<<endl;
    return ;
  }
  string line;
  vector<pair<uint32_t,char>> trace;
  int a=0;
  while(getline(infile, line))
  {
    std::stringstream ss(line);
    char operation;
    std::string hex_str;
    uint32_t hex_value;
    ss >> operation >> hex_str;

    // Convert the hexadecimal string to a uint32_t integer
    std::stringstream hex_stream;
    hex_stream << std::hex << hex_str;
    hex_stream >> hex_value;
    //cout<<hex_value;
    trace.push_back(make_pair(hex_value,operation));
    if(trace[a].second != 'r' && trace[a].second != 'w')
    exit(1);
    //cout<<std::hex<<trace[a].first<<endl;
    a++;
  }
  int reads = 0;
  int writes = 0;
  int l1reads=0;
  int l1writes =0;
  int l1readmisses = 0;
  int l1writemisses = 0;
  int l2reads = 0;
  int l2writes = 0;
  int l2readmisses = 0;
  int l2writemisses = 0;
  int l2writebacks = 0;
  int l1writebacks = 0;
  for(int i =0;i<trace.size();i++)
  {
    /*
    Scenarios : L1 hit, L1miss L2 hit, L2 miss
    */
   uint32_t set_value = (trace[i].first>> L1.block_offset) & ((1 << L1.setsbits) - 1);

   uint32_t tag_value = trace[i].first >> (L1.block_offset + L1.setsbits);
   uint32_t l2setvalue = (trace[i].first>> L2.block_offset) & ((1 << L2.setsbits) - 1);
   uint32_t l2tagvalue = trace[i].first >> (L2.block_offset + L2.setsbits);
   //cout<<hex<<trace[i].first<<endl;
   //cout<<"L1 check"<<endl;
   if(trace[i].second == 'r')
   l1reads++;
   else
   l1writes++;
   if(!(L1.checkhit(trace[i].first, trace[i].second)))
   {
    //L1 miss
    //Eviction of the block
    if(trace[i].second == 'r')
    l1readmisses++;
    else
    l1writemisses++;
    pair<uint32_t,char> p = L1.misshandle(trace[i].first,trace[i].second);
    if(p.second == 'D')
    {
      //write back
      l1writebacks++;
      l2writes++;
      if(!(L2.checkhit(p.first, p.second)))
      {pair<uint32_t, char> l = L2.misshandle(p.first, p.second);
      l2writemisses++;
      if(l.second == 'D')
      l2writebacks++;
      }
      //L1 eviction and add the block to L2
    }
    //Then check if L2 has the block
    char prev = ' ';
    if(L2.cachememory.find(l2setvalue) != L2.cachememory.end() && L2.cachememory[l2setvalue].find(l2tagvalue) != L2.cachememory[l2setvalue].end())
    {
      prev = L2.cachememory[l2setvalue][l2tagvalue].second;
    }
    if(!(L2.checkhit(trace[i].first, trace[i].second)))
    {
      //L2 miss loading from the memory so shouldn't be Dirty
      l2readmisses++;
      l2reads++;
      pair<uint32_t,char> x = L2.misshandle(trace[i].first, ' ');
      if(x.second == 'D')
      l2writebacks++;
    }
    else
    {
      //L2 hit
      //The thing is present, either dirty or not
      //if L2 has the block update wheteher L1 block should be dirty or not
      L2.cachememory[l2setvalue][l2tagvalue].second = prev;
      l2reads++;
      L1.cachememory[set_value][tag_value].second = (trace[i].second == 'r')?' ':'D';
    }
   }
  }
  printstats(false, true,&L1,&L2,NULL,l1size,l1assoc,l1blocksize,l2size,l2assoc,0,s,l1reads,l1readmisses,l1writes,l1writemisses,0,l1writebacks,0,l2reads,l2readmisses,l2writes,l2writemisses,l2writebacks);
}
void L1L2VC(uint32_t l1size, uint32_t l1assoc, uint32_t blocksize, uint32_t vcblocks, uint32_t l2size, uint32_t l2assoc,string s)
{
  Cache L1(l1size,l1assoc, blocksize);
  Cache L2(l2size,l2assoc,blocksize);
  Cache VC(blocksize*vcblocks,vcblocks,blocksize);
  //cout<<"Hi"<<endl;
  ifstream infile(s);
  if(!infile)
  {
    cerr<<"Error Opening File"<<endl;
    return;
  }
  string line;
  vector<pair<uint32_t,char>> trace;
  int a=0;
  int l1reads = 0;
  int l1writes = 0;
  int l1readmiss = 0;
  int l1writemiss = 0;
  int swaprequests =0;
  int swaps =0;
  int l1writebacks = 0;
  int l2reads = 0;
  int l2writes = 0;
  int l2readmiss = 0;
  int l2writemiss = 0;
  int l2writebacks = 0;
  while(getline(infile, line))
  {
    std::stringstream ss(line);
    char operation;
    std::string hex_str;
    uint32_t hex_value;
    ss >> operation >> hex_str;

    // Convert the hexadecimal string to a uint32_t integer
    std::stringstream hex_stream;
    hex_stream << std::hex << hex_str;
    hex_stream >> hex_value;
    //std::cout<<hex_value<<endl;
    trace.push_back(make_pair(hex_value,operation));
    //cout<<std::hex<<trace[a].first<<endl;
    a++;
  }
  for(int i=0;i<trace.size();i++)
  {
    /*
    We have 4 scenarios
    L1 hit
    L1 miss VC hit
    L1 miss VC miss L2 hit( Have to ensure writeback)
    L1 miss VC miss L2 miss
    */
   if(trace[i].second == 'r')
   l1reads++;
   else
   l1writes++;
    bool fl1 = L1.checkhit(trace[i].first, trace[i].second);
    //I have to check whether the set is full or not first
    //If full then only enter victim cache.
    uint32_t block_offset_value = trace[i].first & ((1 << L1.block_offset) - 1);
    uint32_t set_value = (trace[i].first >> L1.block_offset) & ((1 << L1.setsbits) - 1);
    uint32_t tag_value = trace[i].first >> (L1.block_offset + L1.setsbits);
    if(!fl1)
    {
      if(trace[i].second == 'r')
      l1readmiss++;
      else
      l1writemiss++;
      //L1 miss. Check VC only if L1 set size is full otherwise directly proceed to L2
      if(L1.cachememory[set_value].size() == l1assoc)
      {
        swaprequests++;
        //L1 miss and Size of L1 is full that time only access VC
        if(!(VC.checkhit(trace[i].first,trace[i].second)))
        {
          //VC miss have to access L2
          pair<uint32_t,char> p = L1.misshandle(trace[i].first,trace[i].second);
          pair<uint32_t, char> vcout = VC.misshandle(p.first,p.second);
          if(vcout.second == 'D')
          {
            //perform writeback
            l1writebacks++;
            l2writes++;
            if(!(L2.checkhit(vcout.first, vcout.second)))
            {
              l2writemiss++;
              pair<uint32_t, char> l = L2.misshandle(vcout.first, vcout.second);
            if(l.second == 'D')
            l2writebacks++;}
          }
          char prev = ' ';
          uint32_t l2setvalue = (trace[i].first>> L2.block_offset) & ((1 << L2.setsbits) - 1);
          uint32_t l2tagvalue = trace[i].first >> (L2.block_offset + L2.setsbits);
          if(L2.cachememory.find(l2setvalue) != L2.cachememory.end() && L2.cachememory[l2setvalue].find(l2tagvalue) != L2.cachememory[l2setvalue].end())
          {
              prev = L2.cachememory[l2setvalue][l2tagvalue].second;
          }
          if(!(L2.checkhit(trace[i].first, trace[i].second)))
          {
            l2reads++;
            l2readmiss++;
            pair<uint32_t,char> x = L2.misshandle(trace[i].first, ' ');
            if(x.second == 'D')
               l2writebacks++;
          }
          else
          {
            l2reads++;
            L2.cachememory[l2setvalue][l2tagvalue].second = prev;
            L1.cachememory[set_value][tag_value].second = (trace[i].second == 'r')?' ':'D';
          }
        }
        else
        {
          //VC hit this is
        //swap values here so that LRU isn't removed
        //cout<<"LRU of victim before swap"<<endl;
        //printLRUcounter(VC);
        pair<uint32_t, char> p = L1.misshandle(trace[i].first, trace[i].second); // This replcaes the L1 cache line with the new line fron VC and then this replaces the VC line of that trace
        uint32_t block_offset_valuevc = p.first & ((1 << VC.block_offset) - 1);
        uint32_t set_valuevc = (p.first >> VC.block_offset) & ((1 << VC.setsbits) - 1);
        uint32_t tag_valuevc = p.first >> (VC.block_offset + VC.setsbits);
        uint32_t bovc = trace[i].first & ((1 << VC.block_offset) - 1);
        uint32_t svvc = (trace[i].first >> VC.block_offset) & ((1 << VC.setsbits) - 1);
        uint32_t tvvc = trace[i].first >> (VC.block_offset + VC.setsbits);
        if(VC.cachememory[svvc][tvvc].second == 'D')
        L1.cachememory[set_value][tag_value].second = 'D';
        //Updating the LRU of VC
        int val = VC.LRU_counter[svvc][tvvc];
        VC.cachememory[svvc].erase(tvvc);
        VC.LRU_counter[svvc].erase(tvvc);
        for(auto &it : VC.LRU_counter[set_valuevc])
        {
          int lru = it.second;
          uint32_t tag = it.first;
          /*I want to check if two values of same LRU are already there or not and then after updating I want to check*/

          if(lru < val)
          {
            VC.LRU_counter[set_valuevc][tag] = lru+1;
          }
        }
        VC.LRU_counter[set_valuevc][tag_valuevc] = 0;// LRU updation
        //Add the one evicted out from L1 to VC
        VC.cachememory[set_valuevc][tag_valuevc].first = true;
        VC.cachememory[set_valuevc][tag_valuevc].second = p.second;
        //Erase the content of the line which got swapped
        
        //Erase the LRU counter
        //counter has been erased above
        swaps++;
        }
      }
      else if(L1.cachememory[set_value].size() < l1assoc)
      {
        //L1 isn't full so directly fetch from L2 then
        pair<uint32_t,char> p = L1.misshandle(trace[i].first,trace[i].second);
        //pair<uint32_t, char> vcout = VC.misshandle(p.first,p.second);
          if(p.second == 'D')
          {
            //perform writeback
            l1writebacks++;
            l2writes++;
            if(!(L2.checkhit(p.first, p.second)))
              {
                if(p.second == 'D')
                l2writemiss++;
                pair<uint32_t, char> l = L2.misshandle(p.first, p.second);
              if(l.second == 'D')
              l2writebacks++;
              }
          }
          char prev = ' ';
          uint32_t l2setvalue = (trace[i].first>> L2.block_offset) & ((1 << L2.setsbits) - 1);
          uint32_t l2tagvalue = trace[i].first >> (L2.block_offset + L2.setsbits);
          int s= L2.cachememory[set_value].size();
          if(L2.cachememory.find(l2setvalue) != L2.cachememory.end() && L2.cachememory[l2setvalue].find(l2tagvalue) != L2.cachememory[l2setvalue].end())
          {
              prev = L2.cachememory[l2setvalue][l2tagvalue].second;
          }
          if(!(L2.checkhit(trace[i].first, trace[i].second)))
          {
            l2readmiss++;
            l2reads++;
            pair<uint32_t,char> x = L2.misshandle(trace[i].first, ' ');
            if(x.second == 'D')
              l2writebacks++;
          }
          else
          {
            l2reads++;
            L2.cachememory[l2setvalue][l2tagvalue].second = prev;
            L1.cachememory[set_value][tag_value].second = (trace[i].second == 'r')?' ':'D';
          }
      }
      else
      {
        cout<<"Size increased more than assoc"<<endl;
        exit(1);
      }
    }
  }
  printstats(true,true,&L1,&L2,&VC,l1size,l1assoc,blocksize,l2size,l2assoc,vcblocks,s,l1reads,l1readmiss,l1writes,l1writemiss,swaprequests,l1writebacks,swaps,l2reads,l2readmiss,l2writes,l2writemiss,l2writebacks);
}
void printstats(bool vc, bool l2, Cache *L1, Cache *L2, Cache *VC, uint32_t l1size,uint32_t l1assoc, uint32_t blocksize, uint32_t l2size,uint32_t l2assoc, uint32_t vcblocks, string file_name,int l1reads, int l1readmisses, int l1writes, int l1writemisses, int swaprequests,int l1vcwriteback, int swaps, int l2reads, int l2readmisses, int l2writes, int l2writemisses, int l2writebacks)
{
  float L2missrate = 0; 
  cout<<"===== Simulator configuration ====="<<endl;
  cout<<"\tL1_SIZE:\t"<<l1size<<endl;
  cout<<"\tL1_ASSOC:\t"<<l1assoc<<endl;
  cout<<"\tL1_BLOCKSIZE:\t"<<blocksize<<endl;
  cout<<"\tVC_NUM_BLOCKS:\t"<<vcblocks<<endl;
  cout<<"\tL2_SIZE:\t"<<l2size<<endl;
  cout<<"\tL2_ASSOC:\t"<<l2assoc<<endl;
  cout<<"\ttrace_file:\t"<<file_name<<endl;
  cout<<endl;
  cout<<"===== L1 contents ====="<<endl;
  L1->print_cache();
  if(vc)
  {
    cout<<endl;
    cout<<"===== VC contents ====="<<endl;
    VC->print_cache();
  }
  if(l2)
  {
    cout<<endl;
    cout<<"===== L2 contents ====="<<endl;
    L2->print_cache();
  }
  cout<<endl;
  cout<<"===== Simulation results (raw) ====="<<endl;
  cout<<"  a. number of L1 reads:				"<<dec<<l1reads<<endl;
  cout<<"  b. number of L1 read misses:				"<<l1readmisses<<endl;
  cout<<"  c. number of L1 writes:				"<<l1writes<<endl;
  cout<<"  d. number of L1 write misses:				"<<l1writemisses<<endl;
  cout<<"  e. number of swap requests:				"<<swaprequests<<endl;
  cout<<"  f. swap request rate:					"<< round((float)(swaprequests * 1.0 / (l1reads + l1writes))*10000.0)/10000.0 << endl;
  cout<<"  g. number of swaps:					"<<swaps<<endl;
  cout<<"  h. combined L1+VC miss rate:				"<<round((float)((l1readmisses*1.0+l1writemisses*1.0-swaps*1.0)/(l1reads*1.0+l1writes*1.0))*10000.0)/10000.0<<endl;
  cout<<"  i. number writebacks from L1/VC:			"<<l1vcwriteback<<endl;
  cout<<"  j. number of L2 reads:				"<<l2reads<<endl;
  cout<<"  k. number of L2 read misses:				"<<l2readmisses<<endl;
  cout<<"  l. number of L2 writes:				"<<l2writes<<endl;
  cout<<"  m. number of L2 write misses:				"<<l2writemisses<<endl;
  if(l2)
  {cout<<"  n. L2 miss rate:					"<<round(((float)(l2readmisses*1.0 / l2reads))*10000.0)/10000.0<<endl;
  L2missrate = (float)((l2readmisses*1.0 / l2reads));
  }
  else
  {cout<<"  n. L2 miss rate:					"<<0.0000<<endl;
  L2missrate = 0.0f;
  }
  cout<<"  o. number of writebacks from L2:			"<<l2writebacks<<endl;
  if(!l2)
  cout<<"  p. total memory traffic:				"<<(l1readmisses + l1writemisses - swaps + l1vcwriteback)<<endl;
  else
  cout<<"  p. total memory traffic:				"<<(l2readmisses + l2writemisses + l2writebacks)<<endl;
  cout<<endl;
  cout<<"===== Simulation results (performance) ====="<<endl;
  // float L1HT =0.0f;float L1E = 0.0f; float L1A = 0.0f;
  // auto results = get_cacti_results(l1size,blocksize,l1assoc,&L1HT,&L1E,&L1A);
  // if(results != 0)
  // {
  //   cout<<"Cacti failed getting L1 results"<<endl;
  //   exit(1);
  // }
  // float L2HT = 0.0f;
  // float L2E = 0.0f;
  // float L2A = 0.0f;
  // if(l2)
  // {
  //   auto r = get_cacti_results(l2size,blocksize,l2assoc,&L2HT,&L2E,&L2A);
  //   if(r!=0)
  //   {
  //     cout<<"Cacti failed getting L2 results"<<endl;
  //     exit(1);
  //   }
  // }
  // float VCHT = 0.0f;
  // float VCE = 0.0f;
  // float VCA = 0.0f;
  // if(vc)
  // {
  //   auto r = get_cacti_results(VC->size,VC->block_size,VC->assoc,&VCHT,&VCE,&VCA);
  //   if(r!=0)
  //   {
  //     VCHT = 0.2f;
  //     VCE = 0.0f;
  //     VCA = 0.0f;
  //   }
  // }
  // double L1missrate = ((l1readmisses*1.0+l1writemisses*1.0-swaps*1.0)/(l1reads*1.0+l1writes*1.0));
  // if(!l2)
  // L2missrate = 1;
  // double L1misspenalty = (double)(L2HT + (L2missrate)*(20+((blocksize*1.0)/16*1.0)));
  // double AAT = (double)(L1HT + L1missrate*(L1misspenalty) +  (swaprequests * 1.0 / (l1reads + l1writes)) *(VCHT));
  // double Totalenergy;
  // if(l2)
  // Totalenergy =(double)((l1reads + l1writes)*L1E + (l1readmisses + l1writemisses)*L1E + 2*(swaprequests)*VCE + (l2reads + l2writes)*L2E + (l2readmisses+l2writemisses)*L2E + (l2readmisses+ l2writemisses)*0.05f + (l2writebacks)*0.05f);
  // else
  // Totalenergy =(double) ((l1reads + l1writes)*L1E + (l1readmisses + l1writemisses)*L1E + (l1readmisses + l1writemisses - swaps)*0.05 + l1vcwriteback*0.05 + 2*(swaprequests)*VCE);
  // cout<<"  1. average access time:			"<<fixed<<setprecision(4)<<AAT<<endl;
  // cout<<"  2. energy-delay product:			"<<fixed<<(Totalenergy*(l1reads + l1writes)*1.0f*AAT)<<endl;
  // cout<<"  3. total area:				"<<setprecision(4)<<(VCA+L1A+L2A)<<endl;
// Declare L1, L2, and VC variables as float for compatibility with get_cacti_results()
float L1HT = 0.0f, L1E = 0.0f, L1A = 0.0f;
auto results = get_cacti_results(l1size, blocksize, l1assoc, &L1HT, &L1E, &L1A);
if (results != 0) {
    cout << "Cacti failed getting L1 results" << endl;
    exit(1);
}

float L2HT = 0.0f, L2E = 0.0f, L2A = 0.0f;
if (l2) {
    auto r = get_cacti_results(l2size, blocksize, l2assoc, &L2HT, &L2E, &L2A);
    if (r != 0) {
        cout << "Cacti failed getting L2 results" << endl;
        exit(1);
    }
}

float VCHT = 0.0f, VCE = 0.0f, VCA = 0.0f;
if (vc) {
    auto r = get_cacti_results(VC->size, VC->block_size, VC->assoc, &VCHT, &VCE, &VCA);
    if (r != 0) {
        VCHT = 0.2f;
        VCE = 0.0f;
        VCA = 0.0f;
    }
}

// Convert float values to double for higher precision in calculations
double L1HT_d = static_cast<double>(L1HT);
double L1E_d = static_cast<double>(L1E);
double L1A_d = static_cast<double>(L1A);

double L2HT_d = static_cast<double>(L2HT);
double L2E_d = static_cast<double>(L2E);
double L2A_d = static_cast<double>(L2A);

double VCHT_d = static_cast<double>(VCHT);
double VCE_d = static_cast<double>(VCE);
double VCA_d = static_cast<double>(VCA);

// Force constants to be in double precision
double blocksize_d = static_cast<double>(blocksize);
double L1missrate = static_cast<double>((l1readmisses * 1.0 + l1writemisses * 1.0 - swaps * 1.0) / (l1reads * 1.0 + l1writes * 1.0));

if (!l2) {
    L2missrate = 1.0;
}

double L1misspenalty = L2HT_d + L2missrate * (20.0 + (blocksize_d / 16.0));
double AAT = L1HT_d + L1missrate * L1misspenalty + (swaprequests * 1.0 / (l1reads + l1writes)) * VCHT_d;

double Totalenergy;
if (l2) {
    Totalenergy = (l1reads + l1writes) * L1E_d +
                  (l1readmisses + l1writemisses) * L1E_d +
                  2.0 * swaprequests * VCE_d +
                  (l2reads + l2writes) * L2E_d +
                  (l2readmisses + l2writemisses) * L2E_d +
                  (l2readmisses + l2writemisses) * 0.05 + 
                  (l2writebacks) * 0.05;
} else {
    Totalenergy = (l1reads + l1writes) * L1E_d +
                  (l1readmisses + l1writemisses) * L1E_d +
                  (l1readmisses + l1writemisses - swaps) * 0.05 +
                  l1vcwriteback * 0.05 + 
                  2.0 * swaprequests * VCE_d;
}

// Print with high precision
cout << "  1. average access time:            " << fixed << setprecision(4) << AAT << endl;
cout << "  2. energy-delay product:           " << fixed << setprecision(4) << (Totalenergy * static_cast<double>(l1reads + l1writes) * AAT) << endl;
cout << "  3. total area:                     " << fixed << setprecision(4) << (VCA_d + L1A_d + L2A_d) << endl;

}
int main(int argc, char* argv[]) {
    // Check if the correct number of arguments is passed
    if (argc != 8) {
        cerr << "Usage: " << argv[0] << " <l1_size> <l1_assoc> <l1_blocksize> <vc_numblocks> <l2_size> <l2_assoc> <trace_file>" << endl;
        return 1;  // Return with an error code
    }

    // Convert command-line arguments to appropriate types
    uint32_t l1_size = static_cast<uint32_t>(stoi(argv[1]));
    uint32_t l1_assoc = static_cast<uint32_t>(stoi(argv[2]));
    uint32_t l1_blocksize = static_cast<uint32_t>(stoi(argv[3]));
    uint32_t vc_numblocks = static_cast<uint32_t>(stoi(argv[4]));
    uint32_t l2_size = static_cast<uint32_t>(stoi(argv[5]));
    uint32_t l2_assoc = static_cast<uint32_t>(stoi(argv[6]));
    string trace_file = argv[7];
    if(vc_numblocks == 0 && l2_size == 0)
    {
      L1(l1_size,l1_assoc,l1_blocksize,trace_file);
    }
    else if(vc_numblocks !=0 && l2_size == 0)
    {
      Cache L1(l1_size, l1_assoc, l1_blocksize);
      Cache VC(vc_numblocks*l1_blocksize, vc_numblocks, l1_blocksize);
      L1VC(&L1, &VC, l1_assoc, trace_file);
    }
    else if(vc_numblocks ==0 && l2_size != 0)
    {
      L1L2(l1_size,l1_assoc,l1_blocksize,l2_size,l2_assoc,l1_blocksize,trace_file);
    }
    else
    {
      L1L2VC(l1_size,l1_assoc,l1_blocksize,vc_numblocks,l2_size,l2_assoc,trace_file);
    }
 return 0;
}
