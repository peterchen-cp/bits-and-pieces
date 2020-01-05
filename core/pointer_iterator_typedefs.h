#pragma once

/** typedefs for STL-compatible containers, using element pointers as iterators. */
template <typename TElement>
struct pointer_interator_typedefs
{
   using value_type = TElement;
   using reference = TElement & ;
   using const_reference = const TElement &;
   using iterator = TElement * ;
   using const_iterator = const TElement *;
   using difference_type = ptrdiff_t;
   using size_type = size_t;
};
