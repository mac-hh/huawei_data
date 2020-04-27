#include <pthread.h>
#include <sys/types.h>    
#include <sys/stat.h>  
#include <sys/mman.h>  
#include <fcntl.h>
#include <unistd.h>
#include <bits/stdc++.h>

// #define DEBUG_MODE

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)
#define minimum(x, y) x < y ? x : y
#define maximum(x, y) x > y ? x : y
#define u32 unsigned int
#define u8 unsigned char

using namespace std;

const int THREAD_COUNT = 4;                                             // 线程个数
const u32 MAX_VERTEX = 200001;
u32 VERTEX_COUNT = 0;                                                   // 图节点的个数
u32 CYCLE_COUNT[THREAD_COUNT] = {0};                                    // 环的个数
u32 GRAPH[MAX_VERTEX][50], RGRAPH[MAX_VERTEX][50];
u8 GSIZE[MAX_VERTEX], RSIZE[MAX_VERTEX];
char IDS[MAX_VERTEX][10];                                               // mapped_id -> origin_id; idc: id+','; ide: id+'\n'

#define AT3 500000
#define AT4 500000
#define AT5 1000000
#define AT6 2000000
#define AT7 3000000
u32 ANST[8] = {0, 0, 0, AT3, AT4, AT5, AT6, AT7};
u32 ans3[AT3*3*THREAD_COUNT], 
    ans4[AT4*4*THREAD_COUNT], 
    ans5[AT5*5*THREAD_COUNT],
    ans6[AT6*6*THREAD_COUNT], 
    ans7[AT7*7*THREAD_COUNT];
u32 ANSSIZE[THREAD_COUNT][8] = {0};
u32* RESULT[8] = {0, 0, 0, ans3, ans4, ans5, ans6, ans7};

bool mem[MAX_VERTEX];

u8 flag = 0x01;
u32 g_ptr = 0;
u32 tmp = 0x00;
char* ptr;
char chars[7];
u8 charsi;
void build() {
    int fd = open("test_data.txt", O_RDONLY);
    const u32 data_sz = lseek(fd, 0, SEEK_END);
    char* file_ptr = (char*) mmap(                                  // mmap文件指针
        NULL, data_sz, PROT_READ, MAP_PRIVATE, fd, 0
    );

    char* buf = file_ptr;
    const char* buf_end = file_ptr + data_sz;

    
    while(buf < buf_end) {
        if (unlikely(*buf == ',')) {
            VERTEX_COUNT = maximum(VERTEX_COUNT, tmp);
            if (!mem[tmp]) {
                mem[tmp] = true;
                ptr = IDS[tmp];
                u8 i = 0;
                for (; i < charsi; ++i) {
                    ptr[i] = chars[i];
                }
            }
            charsi = 0;

            switch (flag) {
                case 0x01:
                    g_ptr = tmp;
                    break;
                case 0x02:
                    GRAPH[g_ptr][GSIZE[g_ptr]++] = tmp;
                    RGRAPH[tmp][RSIZE[tmp]++] = g_ptr;
                    break;
            }
            flag <<= 1;
            tmp = 0;
        } else if (unlikely(*buf == '\n')) {
            flag = 0x01;
        } else {
            switch (flag) {
                case 0x01:
                case 0x02:
                    chars[charsi++] = *buf;
                    tmp = tmp * 10 + (*buf - '0');
            }
        }

        ++ buf;
    }

    // munmap(file_ptr, data_sz);
    // VERTEX_COUNT += 1;
    for (int i = 0; i < VERTEX_COUNT+1; ++i) {
        sort(GRAPH[i], GRAPH[i]+GSIZE[i]);
    }
}

u8 dist[THREAD_COUNT][MAX_VERTEX];
bool visit[THREAD_COUNT][MAX_VERTEX];
u32 distmem[THREAD_COUNT][MAX_VERTEX / THREAD_COUNT];
u32 distsize[THREAD_COUNT];

void dfs(u32* path, u8 depth, u8 tid) {
    if (path[depth] == path[0]) {
        u32* ptr = &RESULT[depth][ANSSIZE[tid][depth]*depth+tid*ANST[depth]];
        for (u8 si = 0; si < depth; ++si) {
            ptr[si] = path[si];
        }
        ++ ANSSIZE[tid][depth];
        ++ CYCLE_COUNT[tid];
        return;
    }

    if (depth >= 7) return;

    bool* visitn = visit[tid];
    u8* distn = dist[tid];
    u32 node = path[depth];
    u8 nsz = GSIZE[node];
    for (u8 nid = 0; nid < nsz; ++nid) {
        u32 neighbor = GRAPH[node][nid];
        if (!visitn[neighbor] &&
            depth + distn[neighbor] <= 7) {
            path[depth+1] = neighbor;
            visitn[neighbor] = true;
            dfs(path, depth+1, tid);
            visitn[neighbor] = false;
        }
    }
}

void* thread_search(void* ptid) {
    u8 tid = *((u8*)ptid);

    u8* distn = dist[tid];
    bool* visitn = visit[tid];
    u32* distmemn = distmem[tid];
    u32 distsizen = distsize[tid];
    queue<u32> q;
    queue<u8> d;

    memset(distn, 0x0f, (VERTEX_COUNT+1)*sizeof(u8));
    for (u32 node0 = tid; node0 < VERTEX_COUNT+1; node0 += THREAD_COUNT) {
        # ifdef DEBUG_MODE
        if (node0 % 1000 == 0) {
            u32 summ = 0;
            for (u8 i = 0; i < THREAD_COUNT; ++i) {
                summ += CYCLE_COUNT[i];
            }
            cout << "node: " << node0 << " / " << \
                VERTEX_COUNT+1 << "~" << summ << endl;
        }
        #endif
        for (int i = 0; i < distsizen; ++i) {
            distn[distmemn[i]] = 0x0f;
        }
        distsizen = 0;

        // post-order search (bfs)
        q.push(node0);
        d.push(0);
        while (!q.empty()) {
            u32 cur_node = q.front(), cur_dist = d.front();
            q.pop(); d.pop();
            distn[cur_node] = cur_dist;
            distmemn[distsizen++] = cur_node;

            if (cur_dist < 3) {
                auto ptr = RGRAPH[cur_node];
                u8 nsz = RSIZE[cur_node];
                for (u8 nid = 0; nid < nsz; ++nid) {
                    u32 neighbor = RGRAPH[cur_node][nid];
                    if (distn[neighbor] > 3 && neighbor > node0) {
                        q.push(neighbor);
                        d.push(cur_dist+1);
                    }
                }
            }
        }

        // pre-order search (dfs)
        u32 path[8];

        path[0] = node0;
        u8 nsz0 = GSIZE[node0];
        for (u8 nid0 = 0; nid0 < nsz0; ++nid0) {
            u32 node1 = GRAPH[node0][nid0];
            if (node1 <= node0) continue;
            path[1] = node1;
            visitn[node1] = true;
            u8 nsz1 = GSIZE[node1];
            for (u8 nid1 = 0; nid1 < nsz1; ++nid1) {
                u32 node2 = GRAPH[node1][nid1];
                if (node2 <= node0 || node2 == node1) continue;
                path[2] = node2;
                visitn[node2] = true;
                u8 nsz2 = GSIZE[node2];
                for (u8 nid2 = 0; nid2 < nsz2; ++nid2) {
                    u32 node3 = GRAPH[node2][nid2];
                    if (node3 < node0 || node3 == node2 || node3 == node1) continue;
                    path[3] = node3;
                    visitn[node3] = true;
                    dfs(path, 3, tid);
                    visitn[node3] = false;
                }
                visitn[node2] = false;
            }
            visitn[node1] = false;
        }
    }
}

void search() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_t tids[THREAD_COUNT];

    u8 info[THREAD_COUNT];
    for (u8 tc = 0; tc < THREAD_COUNT; ++tc) {
        info[tc] = tc;
        pthread_create(&tids[tc], NULL, thread_search, (void*)&(info[tc]));
    }
    
    pthread_attr_destroy(&attr);
    void *status;
    for (int tc = 0; tc < THREAD_COUNT; ++tc) {
        pthread_join(tids[tc], &status);
    }
}

char buf[3000000*7*7];
u32 bufi;
u32 asz;
char* res;

bool loop(bool* fin) {
    for (u8 i = 0; i < THREAD_COUNT; ++i) {
        if (!fin[i]) return true;
    }
    return false;
}

void save() {
    FILE *fp = fopen("result.txt", "wb");
    u32 CC = 0;
    for (u8 i = 0; i < THREAD_COUNT; ++i) {
        CC += CYCLE_COUNT[i];
    }
    bufi = sprintf(buf, "%d\n", CC);
    u32 cur[THREAD_COUNT];
    bool fin[THREAD_COUNT];
    for (u8 length = 3; length <= 7; ++length) {
        for (int i = 0; i < THREAD_COUNT; ++i) {
            cur[i] = 0;
            fin[i] = ANSSIZE[i][length]==0;
        }
        u32 min_val = 280000;
        u8 min_idx = -1;
        while (loop(fin)) {
            min_val = 2800000;
            for (int i = 0; i < THREAD_COUNT; ++i) {
                if (!fin[i]) {
                    u32 val = RESULT[length][cur[i]*length+i*ANST[length]];
                    if (val < min_val) {
                        min_val = val;
                        min_idx = i;
                    }
                }
            }

            u32* ptr = &RESULT[length][cur[min_idx]*length+min_idx*ANST[length]];
            for (u8 i = 0; i < length - 1; ++i) {
                res = IDS[ptr[i]];
                while (*res) {
                    buf[bufi++] = *res++;
                }
                buf[bufi++] = ',';
            }
            res = IDS[ptr[length-1]];
            while (*res) {
                buf[bufi++] = *res++;
            }
            buf[bufi++] = '\n';

            ++cur[min_idx];
            if (cur[min_idx] >= ANSSIZE[min_idx][length]) {
                fin[min_idx] = true;
            }
        }
    }
    fwrite(buf, bufi, sizeof(char), fp);
    fclose(fp);
}


int main(int argc, char* argv[]){
#ifdef DEBUG_MODE
    clock_t time_start=clock();
    build();
    clock_t time_end=clock();
    cout<<"time use of build: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
    cout << "VERTEX NUMS: " << VERTEX_COUNT+1 << endl;

    time_start=clock();
    search();
    time_end=clock();
    cout<<"time use of search: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
    u32 cc = 0;
    for (u8 i = 0; i < THREAD_COUNT; ++i) {
        cc += CYCLE_COUNT[i];
    }
    cout << "CYCLE NUMS: " << cc << endl;

    time_start=clock();
    save();
    time_end=clock();
    cout<<"time use of save: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;

#else
    build();
    search();
    save();

#endif
}

// clock_t time_start=clock();
// clock_t time_end=clock();
// cout<<"time use:"<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
