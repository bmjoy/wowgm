#pragma once

#include <type_traits>

namespace extstd::containers
{
    template <typename Iterator>
    class ring_iterator final
    {
        using traits_t = std::iterator_traits<Iterator>;

    public:
        using iterator_category = std::forward_iterator_tag;

        using value_t = typename traits_t::value_type;
        using difference_type = typename traits_t::difference_type;
        using pointer = typename traits_t::pointer;
        using reference = typename traits_t::reference;

        explicit ring_iterator() {}

        explicit ring_iterator(Iterator&& itr, Iterator&& end)
            : ring_iterator(itr, itr, end) {
        }

        explicit ring_iterator(Iterator where, Iterator begin, Iterator end)
            : _begin(std::move(begin)), _where(std::move(where)), _end(std::move(end)) {
        }

        ring_iterator& operator ++ () {
            ++_where;
            if (_where == _end)
                _where = _begin;
            return *this;
        }

        ring_iterator operator ++ (int) {
            ring_iterator other(*this);
            ++*this;
            return other;
        }

        ring_iterator& operator -- () {
            --_where;
            if (_where == _begin)
                _where = --_end;
            return *this;
        }

        ring_iterator operator -- (int) {
            ring_iterator other(*this);
            --*this;
            return other;
        }

        reference operator * () const {
            return *_where;
        }

        pointer operator -> () const {
            return _where.operator->();
            // return std::pointer_traits<pointer>::pointer_to(*_where);
        }

    private:
        Iterator _begin;
        Iterator _where;
        Iterator _end;
    };

    template <typename Iterator>
    auto make_ring_iterator(Iterator&& begin, Iterator&& end)
    {
        auto itr = ring_iterator(std::forward<Iterator>(begin), std::forward<Iterator>(end));
        return itr;
    }
}
