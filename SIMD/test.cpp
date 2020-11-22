#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <chrono>

#include "Bitmap.h"
#include "Bloomfilter.h"
#include "CMSketch.h"

using namespace std;
using namespace std::chrono;

const static int MAX_PACKET_NUM = 3e7 + 10;
static int flow[MAX_PACKET_NUM];
int packet_cnt;





void load_data()
{
    BOBHash32 hash_id(rand() % MAX_PRIME32);
    ifstream input("formatted00.dat", ios::in | ios::binary);
    char buf[2000] = {0};
    for(packet_cnt = 0; packet_cnt < MAX_PACKET_NUM; ++packet_cnt)
    {
        if(!input.read(buf, 16)){
            printf("ERROR   %d\n", packet_cnt);
            break;
        }
        flow[packet_cnt] = hash_id.run(buf, 8);
    }
    printf("read in %d packets\n", packet_cnt);
}


void test_bitmap()
{
    Bitmap bitmap;
    unordered_set<int> inSet;
    const static int insertTimesPerUpdate = 10;
    
    /* test accuracy */
    for (int win = (1 << 12); win <= (1 << 14); win <<= 1) 
        for (int mem = (1 << 13); mem <= (1 <<13); mem <<= 1)
        {
            bitmap.init(win, mem);
            for(int i = 0; i + 10 <= win * 3; i += insertTimesPerUpdate){   // insert 3 wins
                for(int j = i; j < i + 10; ++j)
                    bitmap.insert(flow[j]);
                bitmap.update(insertTimesPerUpdate);
            }

            for(int iWin = 3; iWin < 15; ++iWin)        // insert a win each time
            {   
                inSet.clear();
                for(int i = iWin * win; i + insertTimesPerUpdate <= (iWin + 1) * win; i += insertTimesPerUpdate){   // insertion
                    for(int j = i; j < i + insertTimesPerUpdate; ++j){
                        bitmap.insert(flow[j]);
                        inSet.insert(flow[j]);
                    }
                    bitmap.update(insertTimesPerUpdate);
                }

                double bmCard = bitmap.query();
                double cr = bmCard / inSet.size();
                printf("query time range (%-6d-%-6d):\trelatedError:%.6lf\tbmCard:%.6lf\trealCard:%lu\n",
                    iWin * win, (iWin + 1) * win, fabs(cr - 1), bmCard, inSet.size());
            }
        }

    /* test throughput */
    int test_cycle = 10;
    for(int iCase = 0; iCase < 3; ++iCase){
        printf("iCase=%d:\t", iCase);
        auto t1 = steady_clock::now();
        for(int i = 0; i < test_cycle; ++i)
            for(int j = 0; j + insertTimesPerUpdate <= packet_cnt; j += insertTimesPerUpdate)
            {
                for(int k = j; k < j + insertTimesPerUpdate; ++k)
                    bitmap.insert(flow[k]);
                bitmap.update(insertTimesPerUpdate);
            }
        auto t2 = steady_clock::now();
        auto t3 = duration_cast<microseconds>(t2 - t1).count();
        printf("throughput: %.6lf Mips\n", packet_cnt / (1.0 * t3 / test_cycle));
    }
}

void test_bloomfilter()
{
    BloomFilter bf;
    const static int insertTimesPerUpdate = 10;

    for(int win = (1 << 16); win <= (1 << 16); win <<= 1)
        for(int mem = (1 << 13); mem <= (1 << 18); mem <<= 1)
        {
            int hashnum = 1 + (0.6931 * mem * 8) / (win * 8);
            bf.init(win, mem, hashnum);
            
            /* test throughput */
            int test_cycle = 10;
            for(int iCase = 0; iCase < 3; ++iCase){
                printf("iCase=%d:\t", iCase);
                auto t1 = steady_clock::now();
                for(int i = 0; i < test_cycle; ++i)
                    for(int j = 0; j + insertTimesPerUpdate <= packet_cnt; j += insertTimesPerUpdate)
                    {
                        for(int k = j; k < j + insertTimesPerUpdate; ++k)
                            bf.insert(flow[k]);
                        bf.update(insertTimesPerUpdate);
                    }
                auto t2 = steady_clock::now();
                auto t3 = duration_cast<microseconds>(t2 - t1).count();
                printf("throughput: %.6lf Mips\n", packet_cnt / (1.0 * t3 / test_cycle));
            }
        }
} 

void test_cmsketch()
{
    CMSketch cms;
    const static int insertTimesPerUpdate = 10; // insert 10 items and update once
    double error = 0;
    for(int win = (1 << 12); win <= (1 << 12); win <<= 1)
        for(int mem = (1 << 16); mem <= (1 << 16); mem <<= 1)
        {
            int hashnum = 8;
            cms.init(win, hashnum, mem);
            
            /* test throughput */
            int test_cycle = 1;
            for(int iCase = 0; iCase < 3; ++iCase){
                printf("iCase=%d:\t", iCase);
                auto t1 = steady_clock::now();
                for(int i = 0; i < test_cycle; ++i){
                    for(int j = 0; j + insertTimesPerUpdate <= packet_cnt; j += insertTimesPerUpdate)
                    {
                        for(int k = j; k < j + insertTimesPerUpdate; ++k)
                            cms.insert((char*)&flow[k]);
                        cms.update(insertTimesPerUpdate);
                    }
                    map<int,int> mcnt;
                    set<int> scnt;
                    double are = 0;
                    for (int i = packet_cnt - win + 1; i <= packet_cnt; i++){
                        mcnt[flow[i]]++;
                        scnt.insert(flow[i]);
                    }
                    for (set<int>::iterator it = scnt.begin(); it != scnt.end(); it++) {
                        int f=*it;
                        are += abs (cms.query((const char*)&f) / mcnt[*it] - 1);
                    }
                    are /= scnt.size();
                    cout << "ARE of multi thread is " << are << endl;
                }
                auto t2 = steady_clock::now();
                auto t3 = duration_cast<microseconds>(t2 - t1).count();
                printf("throughput: %.6lf Mips\n", packet_cnt / (1.0 * t3 / test_cycle));
            }
            /* calculate ARE at the final moment */
            // pick 10000 different items

        }
} 

int main()
{
    srand(clock());
    load_data();            printf("\n");
    // test_bitmap();       printf("\n\n\n\n\n");
    // test_bloomfilter();     printf("\n\n\n\n\n");
    test_cmsketch();     printf("\n\n\n\n\n");

    return EXIT_SUCCESS;
}