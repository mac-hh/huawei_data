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

const u32 MAX_VERTEX = 200001;
u32 VERTEX_COUNT = 0;                                                   // 图节点的个数
u32 CYCLE_COUNT = 0;                                                    // 环的个数
u32 GRAPH[MAX_VERTEX][50], RGRAPH[MAX_VERTEX][50];
u8 GSIZE[MAX_VERTEX], RSIZE[MAX_VERTEX];
char IDC[MAX_VERTEX][10], IDE[MAX_VERTEX][10];                          // mapped_id -> origin_id; idc: id+','; ide: id+'\n'
u32 ans3[500000*3], ans4[500000*4], ans5[1000000*5], ans6[2000000*6], ans7[3000000*7];
u32 ANSSIZE[8] = {0};
u32* RESULT[8] = {0, 0, 0, ans3, ans4, ans5, ans6, ans7};

vector<u8> dist(VERTEX_COUNT, 0x0f);
vector<bool> visit(MAX_VERTEX, false);

void build() {
    int fd = open("test_data.txt", O_RDONLY);
    const u32 data_sz = lseek(fd, 0, SEEK_END);
    char* file_ptr = (char*) mmap(                                  // mmap文件指针
        NULL, data_sz, PROT_READ, MAP_PRIVATE, fd, 0
    );

    char* buf = file_ptr;
    const char* buf_end = file_ptr + data_sz;

    u8 flag = 0x01;
    u32 g_ptr = 0;
    u32 tmp = 0x00;
    char* ptr;
    char chars[7];
    u8 charsi;
    while(buf < buf_end) {
        if (unlikely(*buf == ',')) {
            VERTEX_COUNT = maximum(VERTEX_COUNT, tmp);
            if (!visit[tmp]) {
                visit[tmp] = true;
                ptr = IDC[tmp];
                u8 i = 0;
                for (; i < charsi; ++i) {
                    ptr[i] = chars[i];
                }
                ptr[i] = ',';
                ptr = IDE[tmp];
                i = 0;
                for (; i < charsi; ++i) {
                    ptr[i] = chars[i];
                }
                ptr[i] = '\n';
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

void dfs(u32* path, u8 depth) {
    if (path[depth] == path[0]) {
        u32* ptr = &RESULT[depth][ANSSIZE[depth]*depth];
        for (u8 si = 0; si < depth; ++si)
            ptr[si] = path[si];
        ++ ANSSIZE[depth];
        // RESULT[depth].emplace_back(move(tmp));
        ++ CYCLE_COUNT;
        return;
    }

    if (depth >= 7) return;

    u32 node = path[depth];
    u8 nsz = GSIZE[node];
    for (u8 nid = 0; nid < nsz; ++nid) {
        u32 neighbor = GRAPH[node][nid];
        if (!visit[neighbor] &&
            depth + dist[neighbor] <= 7) {
            path[depth+1] = neighbor;
            visit[neighbor] = true;
            dfs(path, depth+1);
            visit[neighbor] = false;
        }
    }
}


void search() {
    VERTEX_COUNT += 1;
    for (u32 node0 = 0; node0 < VERTEX_COUNT; ++node0) {
        # ifdef DEBUG_MODE
        if (node0 % 1000 == 0) {
            cout << "node: " << node0 << " / " << \
                VERTEX_COUNT << "~" << CYCLE_COUNT << endl;
        }
        #endif
        dist = vector<u8>(VERTEX_COUNT, 0x0f);
        visit = vector<bool>(VERTEX_COUNT);

        // post-order search (bfs)
        queue<u32> q;
        queue<u8> d;
        q.push(node0);
        d.push(0);
        while (!q.empty()) {
            u32 cur_node = q.front(), cur_dist = d.front();
            q.pop(); d.pop();
            dist[cur_node] = cur_dist;

            if (cur_dist < 3) {
                auto ptr = RGRAPH[cur_node];
                u8 nsz = RSIZE[cur_node];
                for (u8 nid = 0; nid < nsz; ++nid) {
                    u32 neighbor = RGRAPH[cur_node][nid];
                    if (dist[neighbor] > 3 && neighbor > node0) {
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
            visit[node1] = true;
            u8 nsz1 = GSIZE[node1];
            for (u8 nid1 = 0; nid1 < nsz1; ++nid1) {
                u32 node2 = GRAPH[node1][nid1];
                if (node2 <= node0 || node2 == node1) continue;
                path[2] = node2;
                visit[node2] = true;
                u8 nsz2 = GSIZE[node2];
                for (u8 nid2 = 0; nid2 < nsz2; ++nid2) {
                    u32 node3 = GRAPH[node2][nid2];
                    if (node3 < node0 || node3 == node2 || node3 == node1) continue;
                    path[3] = node3;
                    visit[node3] = true;
                    dfs(path, 3);
                    visit[node3] = false;
                }
                visit[node2] = false;
            }
            visit[node1] = false;
        }
    }
}

char buf[3000000*7*7];

void save() {
    FILE *fp = fopen("result.txt", "wb");
    u32 bufi = 0;
    bufi = sprintf(buf, "%d\n", CYCLE_COUNT);
    for (u8 length = 3; length <= 7; ++length) {
        u32 asz = ANSSIZE[length];
        for (u32 aid = 0; aid < asz; ++aid) {
            auto ptr = &RESULT[length][aid*length];
            for (u8 i = 0; i < length-1; ++i) {
                char* res = IDC[ptr[i]];
                while (*res) {
                    buf[bufi++] = *res++;
                }
            }
            char* res = IDE[ptr[length-1]];
            while (*res) {
                buf[bufi++] = *res++;
            }
        }
    }
    fwrite(buf, bufi, sizeof(char), fp);
    fclose(fp);
}


int main(int argc, char* argv[]){
#ifdef DEBUG_MODE
    cout << "program begin!" << endl;
    clock_t time_start=clock();
    build();
    clock_t time_end=clock();
    cout<<"time use of build: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
    cout << "VERTEX NUMS: " << VERTEX_COUNT << endl;

    time_start=clock();
    search();
    time_end=clock();
    cout<<"time use of search: "<<(time_end-time_start)/(double)CLOCKS_PER_SEC<<"s"<<endl;
    cout << "CYCLE NUMS: " << CYCLE_COUNT << endl;

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
