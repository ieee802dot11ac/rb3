/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Copyright (c) 1999
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_INTERNAL_PAIR_H
#define _STLP_INTERNAL_PAIR_H

_STLP_BEGIN_NAMESPACE

    template <class _T1, class _T2>
    struct pair {
        typedef _T1 first_type;
        typedef _T2 second_type;

        _T1 first;
        _T2 second;
#if defined(_STLP_CONST_CONSTRUCTOR_BUG)
        pair() {}
#else
        pair() : first(_T1()), second(_T2()) {}
#endif
        pair(const _T1 &__a, const _T2 &__b) : first(__a), second(__b) {}

#if defined(_STLP_MEMBER_TEMPLATES) && !(defined(_STLP_MSVC) && (_STLP_MSVC < 1200))
        template <class _U1, class _U2>
        pair(const pair<_U1, _U2> &__p) : first(__p.first), second(__p.second) {}

        pair(const pair<_T1, _T2> &__o) : first(__o.first), second(__o.second) {}
#endif
        __TRIVIAL_DESTRUCTOR(pair)
    };

    template <class _T1, class _T2>
    inline bool _STLP_CALL operator==(
        const pair<_T1, _T2> &__x, const pair<_T1, _T2> &__y
    ) {
        return __x.first == __y.first && __x.second == __y.second;
    }

    template <class _T1, class _T2>
    inline bool _STLP_CALL operator<(
        const pair<_T1, _T2> &__x, const pair<_T1, _T2> &__y
    ) {
        return __x.first < __y.first
            || (!(__y.first < __x.first) && __x.second < __y.second);
    }

#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

    template <class _T1, class _T2>
    inline bool _STLP_CALL operator!=(
        const pair<_T1, _T2> &__x, const pair<_T1, _T2> &__y
    ) {
        return !(__x == __y);
    }

    template <class _T1, class _T2>
    inline bool _STLP_CALL operator>(
        const pair<_T1, _T2> &__x, const pair<_T1, _T2> &__y
    ) {
        return __y < __x;
    }

    template <class _T1, class _T2>
    inline bool _STLP_CALL operator<=(
        const pair<_T1, _T2> &__x, const pair<_T1, _T2> &__y
    ) {
        return !(__y < __x);
    }

    template <class _T1, class _T2>
    inline bool _STLP_CALL operator>=(
        const pair<_T1, _T2> &__x, const pair<_T1, _T2> &__y
    ) {
        return !(__x < __y);
    }

#endif /* _STLP_USE_SEPARATE_RELOPS_NAMESPACE */

    // #if defined(_STLP_FUNCTION_TMPL_PARTIAL_ORDER) && !defined(_STLP_NO_EXTENSIONS)          \
//     && !defined(__BORLANDC__) && !defined(__DMC__)
    template <class _T1, class _T2, int _Sz>
    inline pair<_T1, _T2 const *> make_pair(_T1 const &__x, _T2 const(&__y)[_Sz]) {
        return pair<_T1, _T2 const *>(__x, static_cast<_T2 const *>(__y));
    }

    template <class _T1, class _T2, int _Sz>
    inline pair<_T1 const *, _T2> make_pair(_T1 const(&__x)[_Sz], _T2 const &__y) {
        return pair<_T1 const *, _T2>(static_cast<_T1 const *>(__x), __y);
    }

    template <class _T1, class _T2, int _Sz1, int _Sz2>
    inline pair<_T1 const *, _T2 const *> make_pair(
        _T1 const(&__x)[_Sz1], _T2 const(&__y)[_Sz2]
    ) {
        return pair<_T1 const *, _T2 const *>(
            static_cast<_T1 const *>(__x), static_cast<_T2 const *>(__y)
        );
    }
    // #endif

    template <class _T1, class _T2>
    inline pair<_T1, _T2> _STLP_CALL make_pair(_T1 __x, _T2 __y) {
        return pair<_T1, _T2>(__x, __y);
    }

_STLP_END_NAMESPACE

#if defined(_STLP_USE_NAMESPACES) || !defined(_STLP_USE_SEPARATE_RELOPS_NAMESPACE)
_STLP_BEGIN_RELOPS_NAMESPACE

    template <class _Tp>
    inline bool _STLP_CALL operator!=(const _Tp &__x, const _Tp &__y) {
        return !(__x == __y);
    }

    template <class _Tp>
    inline bool _STLP_CALL operator>(const _Tp &__x, const _Tp &__y) {
        return __y < __x;
    }

    template <class _Tp>
    inline bool _STLP_CALL operator<=(const _Tp &__x, const _Tp &__y) {
        return !(__y < __x);
    }

    template <class _Tp>
    inline bool _STLP_CALL operator>=(const _Tp &__x, const _Tp &__y) {
        return !(__x < __y);
    }

_STLP_END_RELOPS_NAMESPACE

#endif

#endif /* _STLP_INTERNAL_PAIR_H */

// Local Variables:
// mode:C++
// End:
