#include <iostream>
#include <sys/types.h>    
#include <sys/stat.h>  
#include <sys/mman.h>  
#include <fcntl.h>
#include <unistd.h>
#include <bits/stdc++.h>

#define DEBUG_MODE

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)
#define minimum(x, y) x < y ? x : y
#define u32 unsigned int
#define u8 unsigned char

using namespace std;

struct Threesome {
    u32 first;
    u32 second;
    u32 third;
};

class Solution {
private:
    u32 VERTEX_COUNT;                                                   // 图节点的个数
    u32 CYCLE_COUNT;                                                    // 环的个数
    vector<u32> ORIGRAPH;                                               // 存储原始edge pair
    vector<vector<u32>> GRAPH;                                          // 存储映射后的邻接表
    unordered_map<u32, u32> MAP;                                        // origin_id -> mapped_id
    vector<string> IDC, IDE;                                            // mapped_id -> origin_id; idc: id+','; ide: id+'\n'
    set<u32> IDSET;                                                     // 存放文件中所有的id
    vector<unordered_map<u32, vector<vector<u32>>>> MEMORY;             // 记录预先搜索长度为2+1+1的链, MEMORY[end][begin][i][j]
    vector<vector<vector<u32>>> RESULT;
    vector<Threesome> PATH;

    void init() {
        VERTEX_COUNT = 0;
        CYCLE_COUNT = 0;
        RESULT = vector<vector<vector<u32>>>(8);
    }

    bool finde(u32 s[], u8 from, u8 to, u32 t) {
        for (; from < to; ++from) {
            if (s[from] == t) return true;
        }
        return false;
    }

    void build() {
        FILE* file=fopen("test_data.txt","r");
        u32 u,v,c;
        vector<u32> inputs;

        while(fscanf(file,"%d,%d,%d",&u,&v,&c)!=EOF){
            IDSET.emplace(u);
            IDSET.emplace(v);
            ORIGRAPH.emplace_back(u);
            ORIGRAPH.emplace_back(v);
        }

        // map graph id to 0 ... n
        IDC.reserve(IDSET.size());
        IDE.reserve(IDSET.size());
        GRAPH = vector<vector<u32>>(IDSET.size());
        for (auto&& element : IDSET) {
            MAP[element] = VERTEX_COUNT++;
            IDC.emplace_back(to_string(element)+',');
            IDE.emplace_back(to_string(element)+'\n');
        }

        int osz = ORIGRAPH.size();
        for (int i = 0; i < osz; i += 2) {
            GRAPH[MAP[ORIGRAPH[i]]].emplace_back(MAP[ORIGRAPH[i+1]]);
        }

        for (auto&& node : GRAPH) {
            sort(node.begin(), node.end());
        }
    }

    void presearch() {
        MEMORY = vector<unordered_map<u32, vector<vector<u32>>>>(VERTEX_COUNT);
        PATH.reserve(VERTEX_COUNT*100);

        for (u32 node1 = 0; node1 < VERTEX_COUNT; ++node1) {
            for (u32& node2 : GRAPH[node1]) {
                for (u32& node3 : GRAPH[node2]) {
                    if (node1 < node2 &&
                        node1 < node3) {
                        Threesome p;
                        p.first = node1;
                        p.second = node2;
                        p.third = node3;
                        PATH.emplace_back(move(p));
                    }
                    for (u32& node4 : GRAPH[node3]) {
                        if (node4 < node3 &&
                            node4 < node2 &&
                            node4 < node1) {
                            MEMORY[node4][node1].emplace_back(
                                vector<u32>({node2, node3})
                            );
                        }
                    }
                }
            }
        }
    }

    void search_dfs(u32 stack[], u8 top) {
        if (stack[top-1] == stack[0]) {
            vector<u32> tmp;
            for (u8 si = 0; si < top-1; ++si)
                tmp.emplace_back(stack[si]);
            RESULT[top-1].emplace_back(move(tmp));
            ++ CYCLE_COUNT;

            return;
        } else {
            auto& candi_path = MEMORY[stack[0]][stack[top-1]];
            for (auto&& path : candi_path) {
                if (!finde(stack, 0, top, path[0]) &&
                    !finde(stack, 0, top, path[1])) 
                {
                    vector<u32> tmp;
                    for (u8 si = 0; si < top; ++si)
                        tmp.emplace_back(stack[si]);
                    tmp.emplace_back(path[0]);
                    tmp.emplace_back(path[1]);

                    RESULT[top+2].emplace_back(move(tmp));
                    ++ CYCLE_COUNT;
                }
            }
        }

        if (top == 5) {
            return;
        }

        vector<u32>& cur = GRAPH[stack[top-1]];
        int csz = cur.size();
        for (int i = 0; i < csz; ++i){
            if (cur[i] >= stack[0] &&
                !finde(stack, 1, top, cur[i])){
                stack[top] = cur[i];
                search_dfs(stack, top+1);
            }
        }
        return;
    }

    void search() {
        u32 stack[5];

        int psz = PATH.size();
        for (u32 i = 0; i < psz; ++i) {
            #ifdef DEBUG_MODE
            if (i % 50000 == 0) {
                cout << "processing: " << (float(i) / psz * 100.) << "% /" \
                     << " ~ " << CYCLE_COUNT << endl;
            }
            #endif
            Threesome& path = PATH[i];
            stack[0] = path.first;
            stack[1] = path.second;
            stack[2] = path.third;
            search_dfs(stack, 3);
        }
    }

    void save() {
        FILE *fp = fopen("result.txt", "wb");
        char buf[1024];
        int idx = sprintf(buf, "%d\n", CYCLE_COUNT);
        buf[idx] = '\0';
        fwrite(buf, idx , sizeof(char), fp);

        for (u8 length = 3; length <= 7; ++length) {
            for (auto&& ans : RESULT[length]) {
                for (u8 i = 0; i < length-1; ++i) {
                    auto& res = IDC[ans[i]];
                    fwrite(res.c_str(), res.size() , sizeof(char), fp);
                }
                auto& res=IDE[ans[length-1]];
                fwrite(res.c_str(), res.size() , sizeof(char), fp);
            }
        }
    }

public:
    void run() {
        init();
    #ifdef DEBUG_MODE
        clock_t time_start=clock();
        build();
        clock_t time_end=clock();
        cout<<"time use of build: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
        cout << "VERTEX NUMS: " << VERTEX_COUNT << endl;

        time_start=clock();
        presearch();
        time_end=clock();
        cout<<"time use of presearch: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;

        time_start=clock();
        search();
        time_end=clock();
        cout<<"time use of search: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
        cout << "CYCLE NUMS: " << CYCLE_COUNT << endl;

        time_start=clock();
        save();
        time_end=clock();
        cout<<"time use of save: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;

        // int* b = NULL;
        // *b = 1;
        exit(0);
    #else
        build();
        presearch();
        search();
        save();
        // int* b = NULL;
        // *b = 1;
        exit(0);
    #endif
    }
};

int main(int argc, char* argv[]){
    Solution sol;
    sol.run();
}

// clock_t time_start=clock();
// clock_t time_end=clock();
// cout<<"time use:"<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
