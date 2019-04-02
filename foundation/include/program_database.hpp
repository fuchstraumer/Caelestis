#pragma once
#ifndef FOUNDATION_PROGRAM_DATABASE_HPP
#define FOUNDATION_PROGRAM_DATABASE_HPP

struct database_type_info
{   
    // Give each type registered a unique hash code
    size_t HashCode{ 0u };
    // sizeof(YOUR_TYPE)
    size_t Size{ 0u };
    // alignof(YOUR_TYPE)
    size_t Alignment{ 0u };
    // destructor
    void (*DestructorFn)(void* instance_addr);
};

#endif //!FOUNDATION_PROGRAM_DATABASE_HPP
