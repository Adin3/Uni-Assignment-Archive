#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <fstream>
#include <atomic>
#include <pthread.h>
#include <cstdlib>
#include <regex>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <iterator>
#include <set>

struct Files {
    std::string name;
    int id;
    int size;
};

struct Words {
    std::string word;
    int id;

    bool operator==(const Words& other) const {
        return word == other.word && id == other.id;
    }
};

struct WordsHash {
    std::size_t operator()(const Words& w) const {
        return std::hash<std::string>()(w.word) ^ std::hash<int>()(w.id);
    }
};

struct CompareFiles {
    bool operator()(const Files& a, const Files& b) {
        return a.size < b.size;
    }
};

bool isnotalpha(char c) {
    return !std::isalpha(c);
}

bool sortMap(
    const std::pair<std::string, std::set<int>>& a,
    const std::pair<std::string, std::set<int>>& b) {
    if (a.second.size() != b.second.size())
        return a.second.size() > b.second.size();
    return a.first < b.first;
}

struct Map {
    std::priority_queue<Files, std::vector<Files>, CompareFiles>* pri_que;
    std::vector<std::vector<Words>>* words;
    pthread_mutex_t* file_lock;
    pthread_barrier_t* map_finished;
    int thread_id;
};

void* map(void* arg) {
    Map* m = (Map*)arg;
    while (true) {
        Files file;

        pthread_mutex_lock(m->file_lock);
        if (m->pri_que->empty()) {
            pthread_mutex_unlock(m->file_lock);
            break;
        }
        file = m->pri_que->top();
        m->pri_que->pop();
        pthread_mutex_unlock(m->file_lock);

        std::ifstream f(file.name);
        std::string line;
        std::unordered_set<Words, WordsHash> unique_words;
        while (f >> line) {
            Words w;

            line.erase(std::remove_if(line.begin(), line.end(), isnotalpha),
             line.end());

            std::transform(line.begin(), line.end(), line.begin(), tolower);

            w.word = line;
            w.id = file.id;

            auto& vec = (*m->words)[m->thread_id];
            if (unique_words.insert(w).second) {
                if (!w.word.empty()) {
                    vec.push_back(w);
                }
            }
        }
    }
    pthread_barrier_wait(m->map_finished);
    return NULL;
}

struct Reduce {
    std::vector<std::vector<Words>>* words;
    std::vector<std::map<std::string, std::set<int>>>* agregation_map;
    std::vector<std::vector<std::pair<std::string, std::set<int>>>>* sorted_map;
    int thread_id;
    int num_threads;
    std::atomic_char* letter_agregation;
    std::atomic_char* letter;
    std::atomic_char* letter_sort;
    pthread_barrier_t* start_reducing;
    pthread_barrier_t* agregation_finished;
    pthread_barrier_t* map_sorted;
};

void *reduce(void* arg) {
    Reduce* r = (Reduce*)arg;
    pthread_barrier_wait(r->start_reducing);

    /// Agregation

    char temp_letter_agregation;
    while (true) {
        temp_letter_agregation = r->letter_agregation->fetch_add(1);
        if (temp_letter_agregation > 'z') break;
        int agregation_index = temp_letter_agregation - 'a';
        int len = r->words->size();
        for (int i = 0; i < len; i++) {
            for (auto& w : (*r->words)[i]) {
                if (w.word[0] == temp_letter_agregation) {
                    (*r->agregation_map)[agregation_index][w.word].insert(w.id);
                }
            }
        }
    }

    pthread_barrier_wait(r->agregation_finished);

    /// Sorting

    char temp_letter_sort;
    while (true) {
        temp_letter_sort = r->letter_sort->fetch_add(1);
        if (temp_letter_sort > 'z') break;
        int sort_index = temp_letter_sort - 'a';
        std::vector<std::pair<std::string, std::set<int>>> temp_map(
            (*r->agregation_map)[sort_index].begin(),
            (*r->agregation_map)[sort_index].end());

        sort(temp_map.begin(), temp_map.end(), sortMap);

        (*r->sorted_map)[sort_index] = temp_map;
    }

    pthread_barrier_wait(r->map_sorted);

    /// Writing

    char temp_letter;
    while (true) {
        temp_letter = r->letter->fetch_add(1);
        if (temp_letter > 'z') break;
        int sort_index = temp_letter - 'a';
    
        std::string path;
        path += temp_letter;
        path += ".txt";

        std::ofstream g(path);

        auto& map = (*r->sorted_map)[sort_index];

        for (auto& element : map) {
            g << element.first << ":[";
            for (auto it = element.second.begin(); it != element.second.end(); ++it) {
                g << *it;
                if (std::next(it) != element.second.end()) {
                    g << " ";
                }
            }
            g << "]\n";
        }
        g.close();
    }

    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Not enough parameters. <M> <R> <FILE>" << std::endl;
        return -1;
    }

    int const NR_THREADS_MAP = atoi(argv[1]);
    int const NR_THREADS_REDUCE = atoi(argv[2]);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, NR_THREADS_MAP + NR_THREADS_REDUCE);

    pthread_barrier_t barrier_reduce;
    pthread_barrier_init(&barrier_reduce, NULL, NR_THREADS_REDUCE);

    pthread_barrier_t barrier_sort;
    pthread_barrier_init(&barrier_sort, NULL, NR_THREADS_REDUCE);

    std::atomic_char letter_agregation = 'a';
    std::atomic_char letter = 'a';
    std::atomic_char letter_sort = 'a';
    pthread_t threads[NR_THREADS_MAP + NR_THREADS_REDUCE];
    
    std::priority_queue<Files, std::vector<Files>, CompareFiles> pri_que;

    std::ifstream f(argv[3]);
    if (!f.is_open()) {
        std::cout << "Could not open the file - '" << argv[3] << std::endl;
        return -1;
    }

    int id = 1;
    std::string line;
    int number_of_files;
    f >> number_of_files;

    while (f >> line) {
        Files file;
        file.name = line;
        file.id = id++;
        std::filesystem::path filePath = "./" + line;

        if (std::filesystem::exists(filePath)) {
            file.size = std::filesystem::file_size(filePath);
            pri_que.push(file);
        } else {
            std::cout << "File does not exist - " << filePath << std::endl;
        }
    }
    f.close();

    std::vector<std::vector<Words>> words(NR_THREADS_MAP);
    std::vector<std::map<std::string, std::set<int>>> agregation_map(26);
    std::vector<std::vector<std::pair<std::string, std::set<int>>>> sorted_map(26);

    for (int i = 0; i < NR_THREADS_REDUCE + NR_THREADS_MAP; i++) {
        void* (*fun)(void*);
        void* e;

        if (i < NR_THREADS_MAP) {
            Map* mapData = new Map;
            mapData->pri_que = &pri_que;
            mapData->words = &words;
            mapData->file_lock = &mutex;
            mapData->thread_id = i;
            mapData->map_finished = &barrier;
            e = (void*)(mapData);
            fun = map;
        } else {
            Reduce* reduceData = new Reduce;
            reduceData->agregation_map = &agregation_map;
            reduceData->words = &words;
            reduceData->thread_id = i;
            reduceData->num_threads = NR_THREADS_REDUCE;
            reduceData->start_reducing = &barrier;
            reduceData->agregation_finished = &barrier_reduce;
            reduceData->letter = &letter;
            reduceData->letter_sort = &letter_sort;
            reduceData->map_sorted = &barrier_sort;
            reduceData->letter_agregation = &letter_agregation;
            reduceData->sorted_map = &sorted_map;
            e = (void*)(reduceData);
            fun = reduce;
        }

        if (pthread_create(&threads[i], NULL, fun, e)) {
            std::cout << "Error creating thread - " << i << std::endl;
            return -1;
        }
    }

    for (int i = 0; i < NR_THREADS_MAP + NR_THREADS_REDUCE; i++) {
        if (pthread_join(threads[i], NULL)) {
            std::cout << "Error joining thread - " << i << std::endl;
            return -1;
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&barrier_reduce);
    pthread_barrier_destroy(&barrier_sort);

    return 0;
}
