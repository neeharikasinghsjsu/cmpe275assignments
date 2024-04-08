#ifndef SHARED_MEM_H
#define SHARED_MEM_H

// our key (unique name) to the shared memory
#define APP_SHM_KEY 1111

int createSharedMemoryForFilePaths(const std::vector<std::string> &filepaths);
std::vector<std::string> getSharedMemoryFilePaths(char *shmaddr);

#endif