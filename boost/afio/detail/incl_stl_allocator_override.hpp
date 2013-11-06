    //! Stop the default std::vector<> doing unaligned storage
    template<> class vector<BOOST_AFIO_TYPE_TO_BE_OVERRIDEN_FOR_STL_ALLOCATOR_USAGE, allocator<BOOST_AFIO_TYPE_TO_BE_OVERRIDEN_FOR_STL_ALLOCATOR_USAGE>> : public vector<BOOST_AFIO_TYPE_TO_BE_OVERRIDEN_FOR_STL_ALLOCATOR_USAGE, boost::afio::detail::aligned_allocator<BOOST_AFIO_TYPE_TO_BE_OVERRIDEN_FOR_STL_ALLOCATOR_USAGE>>
    {
        typedef vector<BOOST_AFIO_TYPE_TO_BE_OVERRIDEN_FOR_STL_ALLOCATOR_USAGE, boost::afio::detail::aligned_allocator<BOOST_AFIO_TYPE_TO_BE_OVERRIDEN_FOR_STL_ALLOCATOR_USAGE>> Base;
    public:
        explicit vector (const allocator_type& alloc = allocator_type()) : Base(alloc) { }
        explicit vector (size_type n) : Base(n) { }
        vector (size_type n, const value_type& val,
            const allocator_type& alloc = allocator_type()) : Base(n, val, alloc) { }
        template <class InputIterator> vector (InputIterator first, InputIterator last,
            const allocator_type& alloc = allocator_type()) : Base(first, last, alloc) { }
        vector (const vector& x) : Base(x) { }
        //vector (const vector& x, const allocator_type& alloc) : Base(x, alloc) { }
        vector (vector&& x) : Base(std::move(x)) { }
        //vector (vector&& x, const allocator_type& alloc) : Base(std::move(x), alloc) { }
#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
        vector (initializer_list<value_type> il, const allocator_type& alloc = allocator_type()) : Base(il, alloc) { }
#endif
    };