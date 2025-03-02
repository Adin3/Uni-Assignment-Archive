#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

struct SeedPeer {
    std::vector<int> seed;
    std::vector<int> peer;
};

struct File {
    std::string name;
    int nr_hashes;
    std::vector<std::string> hashes;
};

struct Download {
    int rank;
    std::vector<std::string> wish_files;
    std::unordered_map<std::string, File>* own_files;
    std::unordered_map<std::string, pthread_mutex_t>* mutexes;
};

struct Upload {
    int rank;
    std::unordered_map<std::string, File>* own_files;
    std::unordered_map<std::string, pthread_mutex_t>* mutexes;
};

void *download_thread_func(void *arg)
{
    Download d = *(Download*) arg;
    SeedPeer oth_clients;
    int type;

    while (!d.wish_files.empty()) {
        /// TALKING WITH TRACKER
        std::string file = d.wish_files.front();
        d.wish_files.erase(d.wish_files.begin());

        type = 1;
        MPI_Send(&type, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

        int len = file.size();
        MPI_Send(&len, 1, MPI_INT, TRACKER_RANK, 1, MPI_COMM_WORLD);
        MPI_Send(file.c_str(), len, MPI_CHAR, TRACKER_RANK, 2, MPI_COMM_WORLD);

        int seed_size;
        MPI_Recv(&seed_size, 1, MPI_INT, TRACKER_RANK, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int seed[seed_size];
        MPI_Recv(seed, seed_size, MPI_INT, TRACKER_RANK, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        oth_clients.seed.insert(oth_clients.seed.end(), seed, seed + seed_size);

        int peer_size;
        MPI_Recv(&peer_size, 1, MPI_INT, TRACKER_RANK, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int peer[peer_size];
        MPI_Recv(peer, peer_size, MPI_INT, TRACKER_RANK, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        oth_clients.peer.insert(oth_clients.peer.end(), peer, peer + peer_size);

        int hash_count;
        MPI_Recv(&hash_count, 1, MPI_INT, TRACKER_RANK, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        File file_download;
        file_download.name.append(file);
        file_download.nr_hashes = hash_count;
        std::vector<std::string> hashes;
        int segments_downloaded = 0;

        for (int i = 0; i < hash_count; i++) {
            char hash[33];
            MPI_Recv(hash, HASH_SIZE, MPI_CHAR, TRACKER_RANK, 8, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            hash[32] = '\0';
            hashes.push_back(hash);
        }

        file_download.hashes.resize(hash_count);
        (*d.own_files)[file] = file_download;
        /// TALKING WITH TRACKER

        /// TALKING TO PEERS
        int next = 0;

        while (true) {

            int peer;
            if (next < (int)oth_clients.seed.size()) {
                peer = oth_clients.seed[next];
            } else {
                peer = oth_clients.peer[next - oth_clients.seed.size()];
            }

            type = 0;
            MPI_Ssend(&type, 1, MPI_INT, peer, 30, MPI_COMM_WORLD);

            int allow;
            MPI_Recv(&allow, 1, MPI_INT, peer, 31, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (allow) {
                type = 1;
                MPI_Ssend(&type, 1, MPI_INT, peer, 11, MPI_COMM_WORLD);

                MPI_Ssend(hashes[segments_downloaded].c_str(), HASH_SIZE, MPI_CHAR, peer, 12, MPI_COMM_WORLD);
                MPI_Ssend(file.c_str(), MAX_FILENAME, MPI_CHAR, peer, 13, MPI_COMM_WORLD);

                int exists;
                MPI_Recv(&exists, 1, MPI_INT, peer, 14, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (exists) {
                    pthread_mutex_lock(&(*d.mutexes)[file]);
                    (*d.own_files)[file].hashes[segments_downloaded] = hashes[segments_downloaded];
                    segments_downloaded++;
                    pthread_mutex_unlock(&(*d.mutexes)[file]);
                }

                /// UPDATING THE SWARM
                if (segments_downloaded % MAX_FILES == 0) {
                    type = 1;
                    MPI_Send(&type, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

                    int len = file.size();
                    MPI_Send(&len, 1, MPI_INT, TRACKER_RANK, 1, MPI_COMM_WORLD);
                    MPI_Send(file.c_str(), len, MPI_CHAR, TRACKER_RANK, 2, MPI_COMM_WORLD);

                    int seed_size;
                    MPI_Recv(&seed_size, 1, MPI_INT, TRACKER_RANK, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    int seed[seed_size];
                    MPI_Recv(seed, seed_size, MPI_INT, TRACKER_RANK, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    oth_clients.seed.clear();
                    oth_clients.seed.insert(oth_clients.seed.end(), seed, seed + seed_size);

                    int peer_size;
                    MPI_Recv(&peer_size, 1, MPI_INT, TRACKER_RANK, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    int peer[peer_size];
                    MPI_Recv(peer, peer_size, MPI_INT, TRACKER_RANK, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    oth_clients.peer.clear();
                    oth_clients.peer.insert(oth_clients.peer.end(), peer, peer + peer_size);
                }

                type = 1;
                MPI_Ssend(&type, 1, MPI_INT, peer, 30, MPI_COMM_WORLD);
            }
            ///

            if (segments_downloaded == (int)(*d.own_files)[file].hashes.size())
                break;

            next = (next + 1) % (oth_clients.seed.size() + oth_clients.peer.size());
        }
        /// TALKING WITH PEERS

        /// OUTPUT FILE
        std::string output_name;
        output_name += "client";
        output_name += std::to_string(d.rank);
        output_name += "_";
        output_name += file;
        std::ofstream g(output_name);

        for (auto it = (*d.own_files)[file].hashes.begin();  it != (*d.own_files)[file].hashes.end() - 1; ++it) {
            g << *it << '\n';
        }

        g << (*d.own_files)[file].hashes[(*d.own_files)[file].nr_hashes-1];

        oth_clients.seed.clear();
        oth_clients.peer.clear();
        /// OUTPUT FILE
    }

    /// DONE DOWNLOADING
    type = 0;
    MPI_Send(&type, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);
    return NULL;
}

void *upload_thread_func(void *arg)
{
    Upload u = *(Upload *)arg;
    MPI_Request request_type;
    MPI_Status status_type;
    MPI_Request request_index;
    MPI_Status status_index;
    MPI_Request request_file;
    MPI_Status status_file;
    int type;

    while (true) {
        MPI_Irecv(&type, 1, MPI_INT, MPI_ANY_SOURCE, 11, MPI_COMM_WORLD, &request_type);

        MPI_Wait(&request_type, &status_type);
            if (type == 0) {
                break;
            }

        char* hash = new char[33];
        MPI_Irecv(hash, HASH_SIZE, MPI_CHAR, status_type.MPI_SOURCE, 12, MPI_COMM_WORLD, &request_index);
        MPI_Wait(&request_index, &status_index);
        hash[32] = '\0';

        char* file = new char[MAX_FILENAME];
        MPI_Irecv(file, MAX_FILENAME, MPI_CHAR, status_type.MPI_SOURCE, 13, MPI_COMM_WORLD, &request_file);
        MPI_Wait(&request_file, &status_file);

        int exists = 0;
        for (int i = 0; i < (*u.own_files)[file].nr_hashes; i++) {
            pthread_mutex_lock(&(*u.mutexes)[file]);

            if (strncmp((*u.own_files)[file].hashes[i].c_str(), hash, HASH_SIZE) == 0) {
                exists = 1;
                pthread_mutex_unlock(&(*u.mutexes)[file]); 
                break;
            }
            pthread_mutex_unlock(&(*u.mutexes)[file]);
        }

        MPI_Ssend(&exists, 1, MPI_INT, status_type.MPI_SOURCE, 14, MPI_COMM_WORLD);

        delete[] hash;
        delete[] file;
    }

    return NULL;
}

void *ready_queue_thread_func(void *arg)
{
    MPI_Request request_type;
    MPI_Status status_type;
    int type;
    int max_downloaders = 2;
    int cur_downloaders = 0;

    while (true) {
        
        MPI_Irecv(&type, 1, MPI_INT, MPI_ANY_SOURCE, 30, MPI_COMM_WORLD, &request_type);

        MPI_Wait(&request_type, &status_type);

        int allow;
        switch (type) {
            case 0:
                if (cur_downloaders + 1 <= max_downloaders) {
                    cur_downloaders++;
                    allow = 1;
                    MPI_Send(&allow, 1, MPI_INT, status_type.MPI_SOURCE, 31, MPI_COMM_WORLD);
                } else {
                    allow = 0;
                    MPI_Send(&allow, 1, MPI_INT, status_type.MPI_SOURCE, 31, MPI_COMM_WORLD);
                }
                break;
            case 1:
                cur_downloaders--;
                break;
            case 2:
                return NULL;
        }
    }

    return NULL;
}

void tracker(int numtasks, int rank) {
    std::unordered_map<std::string, File> file_info;
    std::unordered_map<std::string, SeedPeer> file_swarn;

    /// Init
    {
        MPI_Status status;
        int finished = 0;
        bool init_done = false;

        while (!init_done) {
            int type;
            MPI_Recv(&type, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

            switch (type) {
                case 0:
                    finished++;
                    if (finished == numtasks-1)
                        init_done = true;
                    break;
                case 1:
                    int len;
                    MPI_Recv(&len, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    char* temp = new char[len + 1];
                    MPI_Recv(temp, len, MPI_CHAR, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    temp[len] = '\0';

                    int hash_count;
                    MPI_Recv(&hash_count, 1, MPI_INT, status.MPI_SOURCE, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    file_info[temp].name.append(temp);
                    file_info[temp].nr_hashes = hash_count;
                    std::vector<std::string> hashes;

                    for (int i = 0; i < hash_count; i++) {
                        char hash[33];
                        MPI_Recv(hash, HASH_SIZE, MPI_CHAR, status.MPI_SOURCE, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        hash[32] = '\0';
                        hashes.push_back(hash);
                    }

                    file_info[temp].hashes = hashes;
                    file_swarn[temp].seed.push_back(status.MPI_SOURCE);

                    delete[] temp;
                    break;
            }
        }

        int broadcast_worked = true;
        MPI_Bcast(&broadcast_worked, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);
    }
    /// Init

    /// Transfer
    {
        MPI_Status status;
        int finished = 0;
        bool transfer_done = false;

        while (!transfer_done) {
            int type;
            MPI_Recv(&type, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            switch (type) {
                case 0: {
                    finished++;
                    if (finished == numtasks-1)
                        transfer_done = true;
                    break;
                }
                case 1: {
                    int len;
                    MPI_Recv(&len, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    char* temp = new char[len + 1];
                    MPI_Recv(temp, len, MPI_CHAR, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    temp[len] = '\0';

                    int seed_size = file_swarn[temp].seed.size();
                    MPI_Send(&seed_size, 1, MPI_INT, status.MPI_SOURCE, 3, MPI_COMM_WORLD);

                    int arr[seed_size];
                    copy(file_swarn[temp].seed.begin(), file_swarn[temp].seed.end(), arr);
                    MPI_Send(arr, seed_size, MPI_INT, status.MPI_SOURCE, 4, MPI_COMM_WORLD);

                    int peer_size = file_swarn[temp].peer.size();
                    MPI_Send(&peer_size, 1, MPI_INT, status.MPI_SOURCE, 5, MPI_COMM_WORLD);

                    int arr_p[peer_size];
                    copy(file_swarn[temp].peer.begin(), file_swarn[temp].peer.end(), arr_p);
                    MPI_Send(arr_p, peer_size, MPI_INT, status.MPI_SOURCE, 6, MPI_COMM_WORLD);

                    if(std::find(file_swarn[temp].peer.begin(), file_swarn[temp].peer.end(), status.MPI_SOURCE) != file_swarn[temp].peer.end()) 
                        break;

                    file_swarn[temp].peer.push_back(status.MPI_SOURCE);
                    MPI_Send(&file_info[temp].nr_hashes, 1, MPI_INT, status.MPI_SOURCE, 7, MPI_COMM_WORLD);

                    for (auto& hash : file_info[temp].hashes) {
                        MPI_Send(hash.c_str(), HASH_SIZE, MPI_CHAR, status.MPI_SOURCE, 8, MPI_COMM_WORLD);
                    }

                    delete[] temp;
                    break;
                }
                case 2:
                    char* file_name = new char[MAX_FILENAME];
                    MPI_Recv(file_name, MAX_FILENAME, MPI_CHAR, status.MPI_SOURCE, 13, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    
                    auto new_end = std::remove(file_swarn[file_name].peer.begin(), file_swarn[file_name].peer.end(), status.MPI_SOURCE);

                    file_swarn[file_name].peer.erase(new_end, file_swarn[file_name].peer.end());
                    file_swarn[file_name].seed.push_back(status.MPI_SOURCE);

                    delete[] file_name;
                    break;
            }
        }

        for (int i = 1; i < numtasks; i++) {
            int type = 0;
            MPI_Ssend(&type, 1, MPI_INT, i, 11, MPI_COMM_WORLD);
        }
    }
    /// Transfer
}

void peer(int numtasks, int rank) {
    pthread_t download_thread;
    pthread_t upload_thread;
    pthread_t ready_queue_thread;
    void *status;
    int r;

    std::unordered_map<std::string, File> own_files;
    std::vector<std::string> wish_files;
    std::unordered_map<std::string, pthread_mutex_t> mutexes;
    /// Init

    /// Input
    {
        std::string path = "in";
        path += std::to_string(rank);
        path += ".txt";
        std::ifstream f(path);

        int nr_own_files; f >> nr_own_files;

        for (int i = 0; i < nr_own_files; i++) {
            File file;
            f >> file.name; f >> file.nr_hashes;

            std::string temp_hash;

            for (int j = 0; j < file.nr_hashes; j++) {
                f >> temp_hash;
                file.hashes.push_back(temp_hash);
            }

            own_files[file.name] = file;

            pthread_mutex_t mutex;
            pthread_mutex_init(&mutex, NULL);
            mutexes[file.name] = mutex;
        }

        int nr_wish_files; f >> nr_wish_files;

        std::string temp_name;

        for (int i = 0; i < nr_wish_files; i++) {
            f >> temp_name;
            wish_files.push_back(temp_name);

            pthread_mutex_t mutex;
            pthread_mutex_init(&mutex, NULL);
            mutexes[temp_name] = mutex;
        }
    }
    /// Input

    /// Send
    {
        int type;

        for (auto& file : own_files) {
            type = 1;
            MPI_Send(&type, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

            int len = file.second.name.size();
            MPI_Send(&len, 1, MPI_INT, TRACKER_RANK, 1, MPI_COMM_WORLD);
            MPI_Send(file.second.name.c_str(), len, MPI_CHAR, TRACKER_RANK, 2, MPI_COMM_WORLD);

            MPI_Send(&file.second.nr_hashes, 1, MPI_INT, TRACKER_RANK, 3, MPI_COMM_WORLD);

            for (auto& hash : file.second.hashes) {
                MPI_Send(hash.c_str(), HASH_SIZE, MPI_CHAR, TRACKER_RANK, 4, MPI_COMM_WORLD);
            }
        }

        type = 0;
        MPI_Send(&type, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);
    }
    /// Send

    /// Start
    {
        int broadcast_worked;
        MPI_Bcast(&broadcast_worked, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);
        if (!broadcast_worked) return;
    }
    /// Start

    /// Init
    Download d;
    d.rank = rank;
    d.wish_files = wish_files;
    d.own_files = &own_files;
    d.mutexes = &mutexes;

    r = pthread_create(&download_thread, NULL, download_thread_func, (void *) &d);
    if (r) {
        printf("Eroare la crearea thread-ului de download\n");
        exit(-1);
    }

    Upload u;
    u.rank = rank;
    u.own_files = &own_files;
    u.mutexes = &mutexes;

    r = pthread_create(&upload_thread, NULL, upload_thread_func, (void *) &u);
    if (r) {
        printf("Eroare la crearea thread-ului de upload\n");
        exit(-1);
    }

    r = pthread_create(&ready_queue_thread, NULL, ready_queue_thread_func, (void *) &rank);
    if (r) {
        printf("Eroare la crearea thread-ului de upload\n");
        exit(-1);
    }

    r = pthread_join(download_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_join(upload_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de upload\n");
        exit(-1);
    }

    int type = 2;
    MPI_Ssend(&type, 1, MPI_INT, rank, 30, MPI_COMM_WORLD);
    
    r = pthread_join(ready_queue_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de upload\n");
        exit(-1);
    }

    for (auto& mutex : mutexes) {
        pthread_mutex_destroy(&mutex.second);
    }
}
 
int main (int argc, char *argv[]) {
    int numtasks, rank;
 
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "MPI nu are suport pentru multi-threading\n");
        exit(-1);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == TRACKER_RANK) {
        tracker(numtasks, rank);
    } else {
        peer(numtasks, rank);
    }

    MPI_Finalize();
}