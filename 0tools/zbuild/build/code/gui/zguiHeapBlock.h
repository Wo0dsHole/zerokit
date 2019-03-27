#ifndef __JUCE_HEAPBLOCK_JUCEHEADER__
#define __JUCE_HEAPBLOCK_JUCEHEADER__

namespace zgui
{

/**
    Very simple container class to hold a pointer to some data on the heap.

    When you need to allocate some heap storage for something, always try to use
    this class instead of allocating the memory directly using malloc/free.

    A HeapBlock<char> object can be treated in pretty much exactly the same way
    as an char*, but as long as you allocate it on the stack or as a class member,
    it's almost impossible for it to leak memory.

    It also makes your code much more concise and readable than doing the same thing
    using direct allocations,

    E.g. instead of this:
    @code
        int* temp = (int*) malloc (1024 * sizeof (int));
        memcpy (temp, xyz, 1024 * sizeof (int));
        free (temp);
        temp = (int*) calloc (2048 * sizeof (int));
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
        free (temp);
    @endcode

    ..you could just write this:
    @code
        HeapBlock <int> temp (1024);
        memcpy (temp, xyz, 1024 * sizeof (int));
        temp.calloc (2048);
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
    @endcode

    The class is extremely lightweight, containing only a pointer to the
    data, and exposes malloc/realloc/calloc/free methods that do the same jobs
    as their less object-oriented counterparts. Despite adding safety, you probably
    won't sacrifice any performance by using this in place of normal pointers.

    The throwOnFailure template parameter can be set to true if you'd like the class
    to throw a std::bad_alloc exception when an allocation fails. If this is false,
    then a failed allocation will just leave the heapblock with a null pointer (assuming
    that the system's malloc() function doesn't throw).

    @see Array, OwnedArray, MemoryBlock
*/
template <class ElementType, bool throwOnFailure = false>
class HeapBlock
{
public:
    //==============================================================================
    /** Creates a HeapBlock which is initially just a null pointer.

        After creation, you can resize the array using the malloc(), calloc(),
        or realloc() methods.
    */
    HeapBlock() throw() :
    data (0)
    {
    }

    /** Creates a HeapBlock containing a number of elements.

        The contents of the block are undefined, as it will have been created by a
        malloc call.

        If you want an array of zero values, you can use the calloc() method or the
        other constructor that takes an InitialisationState parameter.
    */
    explicit HeapBlock (const size_t numElements) :
    data(static_cast<ElementType*>(::HeapAlloc(::GetProcessHeap()/*gHeap*/, 0, numElements * sizeof(ElementType))))
    {
    }

//     /** Creates a HeapBlock containing a number of elements.
// 
//         The initialiseToZero parameter determines whether the new memory should be cleared,
//         or left uninitialised.
//     */
//     HeapBlock (const size_t numElements, const bool initialiseToZero) :
//     data(static_cast<ElementType*>(initialiseToZero ? std::calloc (numElements, sizeof (ElementType)) : std::malloc (numElements * sizeof (ElementType))))
//     {
//     }

    /** Destructor.
        This will free the data, if any has been allocated.
    */
    ~HeapBlock()
    {
        ::HeapFree(::GetProcessHeap()/*gHeap*/, 0, data);
    }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator ElementType*() const throw()                           { return data; }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline ElementType* getData() const throw()                            { return data; }

    /** Returns a void pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator void*() const throw()                                  { return static_cast <void*> (data); }

    /** Returns a void pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator const void*() const throw()                            { return static_cast <const void*> (data); }

    /** Lets you use indirect calls to the first element in the array.
        Obviously this will cause problems if the array hasn't been initialised, because it'll
        be referencing a null pointer.
    */
    inline ElementType* operator->() const  throw()                        { return data; }

    /** Returns a reference to one of the data elements.
        Obviously there's no bounds-checking here, as this object is just a dumb pointer and
        has no idea of the size it currently has allocated.
    */
    template <typename IndexType>
    inline ElementType& operator[] (IndexType index) const throw()         { return data [index]; }

    /** Returns a pointer to a data element at an offset from the start of the array.
        This is the same as doing pointer arithmetic on the raw pointer itself.
    */
    template <typename IndexType>
    inline ElementType* operator+ (IndexType index) const throw()          { return data + index; }

    //==============================================================================
    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator== (const ElementType* const otherPointer) const throw()   { return otherPointer == data; }

    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator!= (const ElementType* const otherPointer) const throw()   { return otherPointer != data; }

    //==============================================================================
    /** Allocates a specified amount of memory.

        This uses the normal malloc to allocate an amount of memory for this object.
        Any previously allocated memory will be freed by this method.

        The number of bytes allocated will be (newNumElements * elementSize). Normally
        you wouldn't need to specify the second parameter, but it can be handy if you need
        to allocate a size in bytes rather than in terms of the number of elements.

        The data that is allocated will be freed when this object is deleted, or when you
        call free() or any of the allocation methods.
    */
    void malloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        ::HeapFree(::GetProcessHeap()/*gHeap*/, 0, data);
        data = static_cast <ElementType*> (::HeapAlloc(::GetProcessHeap()/*gHeap*/, 0, newNumElements * elementSize));
    }

    /** Allocates a specified amount of memory and clears it.
        This does the same job as the malloc() method, but clears the memory that it allocates.
    */
    void calloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        ::HeapFree(::GetProcessHeap()/*gHeap*/, 0, data);
        data = static_cast <ElementType*> (::HeapAlloc(::GetProcessHeap()/*gHeap*/, HEAP_ZERO_MEMORY, newNumElements * elementSize));
    }

    /** Allocates a specified amount of memory and optionally clears it.
        This does the same job as either malloc() or calloc(), depending on the
        initialiseToZero parameter.
    */
    void allocate (const size_t newNumElements, bool initialiseToZero)
    {
        ::HeapFree(::GetProcessHeap()/*gHeap*/, 0, data);
        data = static_cast<ElementType*>(initialiseToZero ? ::HeapAlloc(::GetProcessHeap()/*gHeap*/, HEAP_ZERO_MEMORY, newNumElements * sizeof(ElementType)) : ::HeapAlloc(::GetProcessHeap()/*gHeap*/, 0, newNumElements * sizeof (ElementType)));
    }

    /** Re-allocates a specified amount of memory.

        The semantics of this method are the same as malloc() and calloc(), but it
        uses realloc() to keep as much of the existing data as possible.
    */
    void realloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        data = static_cast<ElementType*>(data == 0 ? ::HeapAlloc(::GetProcessHeap()/*gHeap*/, 0, newNumElements * elementSize) : ::HeapReAlloc(::GetProcessHeap()/*gHeap*/, 0, data, newNumElements * elementSize));
    }

    /** Frees any currently-allocated data.
        This will free the data and reset this object to be a null pointer.
    */
    void free()
    {
        ::HeapFree(::GetProcessHeap()/*gHeap*/, 0, data);
        data = 0;
    }

    /** Swaps this object's data with the data of another HeapBlock.
        The two objects simply exchange their data pointers.
    */
    template <bool otherBlockThrows>
    void swapWith (HeapBlock <ElementType, otherBlockThrows>& other) throw()
    {
        ElementType* temp = data;
        data = other.data;
        other.data = temp;
    }

    /** This fills the block with zeros, up to the number of elements specified.
        Since the block has no way of knowing its own size, you must make sure that the number of
        elements you specify doesn't exceed the allocated size.
    */
    void clear (size_t numElements) throw()
    {
        __stosb((unsigned char*)data, sizeof(ElementType) * numElements);
    }

private:
    ElementType* data;

    ZGUI_DECLARE_NON_COPYABLE(HeapBlock);

    ZGUI_PREVENT_HEAP_ALLOCATION; // Creating a 'new HeapBlock' would be missing the point!
};

}

#endif   // __JUCE_HEAPBLOCK_JUCEHEADER__