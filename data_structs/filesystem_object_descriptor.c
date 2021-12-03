#include "filesystem_object_descriptor.h"


void fileSystemObjectDescriptorSerialize(FileSystemOverviewObjectDescriptor descriptor, uint8_t* buffer)
{
    buffer[0] = descriptor.type;
    buffer[1] = descriptor.nameLength;
    uint32_t* sizeFieldPtr = (void*)(&buffer[2]);
    switch(descriptor.type)
    {
        case FILESYSTEM_OVERVIEW_OBJECT_TYPE_DIR:
            *sizeFieldPtr = descriptor.params.childrenNumber;
            break;
        case FILESYSTEM_OVERVIEW_OBJECT_TYPE_FILE:
            *sizeFieldPtr = descriptor.params.size;
            break;
    }
}
FileSystemOverviewObjectDescriptor fileSystemObjectDescriptorDeserialize(const uint8_t* buffer)
{
    FileSystemOverviewObjectDescriptor descriptor;
    
    descriptor.type = buffer[0];
    descriptor.nameLength = buffer[1];
    uint32_t* sizeFieldPtr = (void*)(&buffer[2]);
    switch(descriptor.type)
    {
        case FILESYSTEM_OVERVIEW_OBJECT_TYPE_DIR:
            descriptor.params.childrenNumber = *sizeFieldPtr;
            break;
        case FILESYSTEM_OVERVIEW_OBJECT_TYPE_FILE:
            descriptor.params.size = *sizeFieldPtr;
            break;
    }
    return descriptor;
}
