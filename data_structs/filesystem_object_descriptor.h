#ifndef FILESYSTEM_OBJECT_DESCRIPTOR_H_
#define FILESYSTEM_OBJECT_DESCRIPTOR_H_

#include <stdint.h>

typedef enum {FILESYSTEM_OVERVIEW_OBJECT_TYPE_DIR, FILESYSTEM_OVERVIEW_OBJECT_TYPE_FILE} FileSystemOverviewObjectType;
typedef struct
{
    FileSystemOverviewObjectType type;
    union
    {        
        uint32_t size; //for file
        uint32_t childrenNumber; // for dir
    }params;
    uint8_t nameLength;
}FileSystemOverviewObjectDescriptor;
#define FILESYSTEM_OBJECT_DESCRIPTOR_SIZE sizeof(FileSystemOverviewObjectDescriptor)
void fileSystemObjectDescriptorSerialize(FileSystemOverviewObjectDescriptor descriptor, uint8_t* buffer);
FileSystemOverviewObjectDescriptor fileSystemObjectDescriptorDeserialize(const uint8_t* buffer);

#endif // FILESYSTEM_OBJECT_DESCRIPTOR_H_

