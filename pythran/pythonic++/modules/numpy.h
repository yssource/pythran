#ifndef PYTHONIC_MODULE_NUMPY_H
#define PYTHONIC_MODULE_NUMPY_H

#include <vector>
#include <cmath>
#include <cstdint>

#define NUMPY_EXPR_TO_NDARRAY0(fname)\
    template<class Op, class Arg, class... Types>\
        auto fname(core::numpy_uexpr<Op,Arg> const& expr, Types... others)\
            -> decltype(fname(typename core::numpy_expr_to_ndarray<core::numpy_uexpr<Op,Arg>>::type(expr), std::forward<Types>(others)...)) \
    {\
        return fname(typename core::numpy_expr_to_ndarray<core::numpy_uexpr<Op,Arg>>::type(expr), std::forward<Types>(others)...);\
    }\
    template<class Op, class Arg0, class Arg1, class... Types>\
        auto fname(core::numpy_expr<Op,Arg0, Arg1> const& expr, Types... others)\
            -> decltype(fname(typename core::numpy_expr_to_ndarray<core::numpy_expr<Op,Arg0,Arg1>>::type(expr), std::forward<Types>(others)...)) \
    {\
        return fname(typename core::numpy_expr_to_ndarray<core::numpy_expr<Op,Arg0, Arg1>>::type(expr), std::forward<Types>(others)...);\
    }

namespace pythonic {
    namespace numpy {

        /* a few classical constants */
        double const pi = 3.141592653589793238462643383279502884;
        double const e = 2.718281828459045235360287471352662498;
        double const nan = std::numeric_limits<double>::quiet_NaN();

        /* numpy standard types */
        namespace proxy {
            // these typedefs are not functions, but the default constructor
            // make it legal to write pythonic::proxy::double_
            // as generated by pythran
            // so we put these typedefs in the proxy namespace
            typedef std::complex<double> complex;
            typedef std::complex<float> complex32;
            typedef std::complex<double> complex64;
            typedef std::complex<long double> complex128;
            typedef float float_;
            typedef float float32;
            typedef double float64;
            typedef double float128;
            typedef double double_;
            typedef int8_t int8;
            typedef int16_t int16;
            typedef int32_t int32;
            typedef int64_t int64;
            typedef uint8_t uint8;
            typedef uint16_t uint16;
            typedef uint32_t uint32;
            typedef uint64_t uint64;
        }


       template<class T, class dtype=typename nested_container_value_type<T>::type>
          core::ndarray<dtype, nested_container_depth<T>::value > array(T&& iterable, dtype d=dtype()) {
              return core::ndarray<dtype, nested_container_depth<T>::value >(std::forward<T>(iterable));
          }

       PROXY(pythonic::numpy, array);

       template<class S, class dtype=double>
          core::ndarray<dtype, std::tuple_size<S>::value> zeros(S&& shape, dtype d=dtype()) {
              return core::ndarray<dtype, std::tuple_size<S>::value>(std::forward<S>(shape), dtype(0));
          }

       template<class dtype=double>
          core::ndarray<dtype, 1> zeros(long size, dtype d=dtype()) {
              return zeros(core::make_tuple(size), d);
          }


       PROXY(pythonic::numpy, zeros);

       template<class S, class dtype=double>
          core::ndarray<dtype, std::tuple_size<S>::value> ones(S&& shape, dtype d=dtype()) {
              return core::ndarray<dtype, std::tuple_size<S>::value>(std::forward<S>(shape), dtype(1));
          }

       template<class dtype=double>
          core::ndarray<dtype, 1> ones(long size, dtype d=dtype()) {
              return ones(core::make_tuple(size), d);
          }

       PROXY(pythonic::numpy, ones);

       template<class S, class dtype=double>
          core::ndarray<dtype, std::tuple_size<S>::value> empty(S&& shape, dtype d=dtype()) {
              return core::ndarray<dtype, std::tuple_size<S>::value>(std::forward<S>(shape), None);
          }
       template<class dtype=double>
          core::ndarray<dtype, 1> empty(long size, dtype d=dtype()) {
              return empty(core::make_tuple(size), d);
          }

       PROXY(pythonic::numpy, empty);

       template<class T, class U, class dtype=long>
           core::ndarray<decltype(std::declval<T>()+std::declval<U>()+std::declval<dtype>()), 1> arange(T begin, U end, dtype step=dtype(1))
           {
               typedef decltype(begin+end+step) R;
               size_t size = std::max(R(0), R(std::ceil((end - begin)/step)));
               core::ndarray<R, 1> a(core::make_tuple((long)size), None);
               if(size)
               {
                   auto prev = a.begin(),
                        end = a.end();
                   *prev = begin;
                   for(auto iter = prev + 1; iter!=end; ++iter) {
                       *iter = *prev + step;
                       prev = iter;
                   }
               }
               return a;
           }

       template<class T>
          core::ndarray<T, 1> arange(T end) {
              return arange(T(0), end);
          }
       PROXY(pythonic::numpy, arange);

       template<class T>
          long alen(T&& expr) {
              return expr.shape[0];
          }
       PROXY(pythonic::numpy, alen);

       core::ndarray<double, 1> linspace(double start, double stop, long num=50, bool endpoint = true)
       {
           double step = (stop - start) / (num - (endpoint?1:0)) ;
           return arange(start, stop + (endpoint?step*.5:0), step);
       }

       PROXY(pythonic::numpy, linspace);

       template<class T, size_t N, class ...S>
           core::ndarray<T, sizeof...(S)> reshape( core::ndarray<T,N> const& expr, S&& ...s) {
               return expr.reshape(core::make_tuple(std::forward<S>(s)...));
           }

       NUMPY_EXPR_TO_NDARRAY0(reshape);

       PROXY(pythonic::numpy, reshape);

       template<class T, size_t N, class dtype=T>
           core::ndarray<dtype,1> cumsum(core::ndarray<T,N> const& expr, dtype d = dtype()) {
               long count = expr.size();
               core::ndarray<dtype,1> cumsumy(core::make_tuple(count), None);
               std::partial_sum(expr.buffer, expr.buffer + count, cumsumy.buffer);
               return cumsumy;
           }

       template<class T, class dtype=T>
           core::ndarray<dtype,1> cumsum(core::ndarray<T,1> const& expr, long axis, dtype d = dtype()) {
               if(axis !=0)
                   throw __builtin__::ValueError("axis out of bounds");
               return cumsum(expr);
           }

       template<class T, size_t N, class dtype=T>
           core::ndarray<dtype,N> cumsum(core::ndarray<T,N> const& expr, long axis, dtype d = dtype()) {
               if(axis<0 || axis >=long(N))
                   throw __builtin__::ValueError("axis out of bounds");

               auto shape = expr.shape;
               core::ndarray<dtype,N> cumsumy(shape, None);
               if(axis==0) {
                   std::copy(expr.buffer, expr.buffer + shape[N-1], cumsumy.buffer);
                   std::transform(cumsumy.begin(), cumsumy.end()-1, expr.begin() + 1, cumsumy.begin() + 1, std::plus<core::ndarray<T,N-1>>());
               }
               else {
                   std::transform(expr.begin(), expr.end(), cumsumy.begin(), [=](core::ndarray<T,N-1> const& e) { return cumsum(e, axis-1, d); });
               }
               return cumsumy;
           }

       PROXY(pythonic::numpy, cumsum);

       template<class T, size_t N>
           T sum(core::ndarray<T,N> const& expr, none_type axis=None) {
               return std::accumulate(expr.buffer, expr.buffer + expr.size(), T(0));
           }

       template<class T>
            T sum( core::ndarray<T,1> const& array, long axis)
            {
                if(axis!=0)
                    throw __builtin__::ValueError("axis out of bounds");
                return sum(array);
            }

       template<class T, size_t N>
            typename core::ndarray<T,N>::value_type sum( core::ndarray<T,N> const& array, long axis)
            {
                if(axis<0 || axis >=long(N))
                    throw __builtin__::ValueError("axis out of bounds");
                auto shape = array.shape;
                if(axis==0)
                {
                    return std::accumulate(array.begin() + 1, array.end(), *array.begin());
                }
                else
                {
                    core::ltuple<long, N-1> shp;
                    std::copy(shape.begin(), shape.end() - 1, shp.begin());
                    core::ndarray<T,N-1> sumy(shp, None);
                    std::transform(array.begin(), array.end(), sumy.begin(), [=](core::ndarray<T,N-1> const& other) {return sum(other, axis-1);});
                    return sumy;
                }
            }

       PROXY(pythonic::numpy, sum);

       template<class E>
           auto min(E&& expr) -> typename std::remove_reference<decltype(expr.at(0))>::type {
               long sz = expr.size();
               if(not sz) 
                   throw __builtin__::ValueError("empty sequence");
               auto res = expr.at(0);
               for(long i = 1; i< sz ; ++i) {
                   auto e_i = expr.at(i);
                   if(e_i< res)
                       res = e_i;
               }
               return res;
           }
       template<class E>
           auto max(E&& expr) -> typename std::remove_reference<decltype(expr.at(0))>::type {
               long sz = expr.size();
               if(not sz) 
                   throw __builtin__::ValueError("empty sequence");
               auto res = expr.at(0);
               for(long i = 1; i< sz ; ++i) {
                   auto e_i = expr.at(i);
                   if(e_i > res)
                       res = e_i;
               }
               return res;
           }

       template<class T>
           T min(core::ndarray<T,1> const& array, long axis) {
               if(axis!=0)
                   throw __builtin__::ValueError("axis out of bounds");
               return min(array);
           }

       template<class T>
           T max(core::ndarray<T,1> const& array, long axis) {
               if(axis!=0)
                   throw __builtin__::ValueError("axis out of bounds");
               return max(array);
           }

       template<class T, size_t N>
           typename core::ndarray<T,N>::value_type min(core::ndarray<T,N> const& array, long axis)
           {
               if(axis<0 || axis >=long(N))
                   throw __builtin__::ValueError("axis out of bounds");
               auto shape = array.shape;
               if(axis==0)
               {
                   core::ltuple<long, N-1> shp;
                   size_t size = 1;
                   for(auto i= shape.begin() + 1, j = shp.begin(); i<shape.end(); i++, j++)
                        size*=(*j = *i);
                   core::ndarray<T,N-1> a(shp, None);
                   auto a_iter = a.buffer;
                   std::copy(array.buffer, array.buffer + size, a_iter);
                   for(auto i = array.begin() + 1; i<array.end(); i++)
                   {
                       auto next_subarray = *i;  //we need this variable to keep this ndarray alive while iter is used
                       auto iter = next_subarray.buffer,
                            iter_end = next_subarray.buffer + next_subarray.size();
                       auto k = a_iter;
                       for(auto j = iter; j<iter_end; j++, k++)
                           *k=std::min(*k,*j);
                    }
                    return a;
               }
               else
               {
                   core::ltuple<long, N-1> shp;
                   std::copy(shape.begin(), shape.end() - 1, shp.begin());
                   core::ndarray<T,N-1> miny(shp, None);
                   std::transform(array.begin(), array.end(), miny.begin(), [=](core::ndarray<T,N-1> const& other) {return min(other, axis-1);});
                   return miny;
               }
           }
       template<class T, size_t N>
           typename core::ndarray<T,N>::value_type max(core::ndarray<T,N> const& array, long axis)
           {
               if(axis<0 || axis >=long(N))
                   throw __builtin__::ValueError("axis out of bounds");
               auto shape = array.shape;
               if(axis==0)
               {
                   core::ltuple<long, N-1> shp;
                   size_t size = 1;
                   for(auto i= shape.begin() + 1, j = shp.begin(); i<shape.end(); i++, j++)
                        size*=(*j = *i);
                   core::ndarray<T,N-1> a(shp, None);
                   auto a_iter = a.buffer;
                   std::copy(array.buffer, array.buffer + size, a_iter);
                   for(auto i = array.begin() + 1; i<array.end(); i++)
                   {
                       auto next_subarray = *i;  //we need this variable to keep this ndarray alive while iter is used
                       auto iter = next_subarray.buffer,
                            iter_end = next_subarray.buffer + next_subarray.size();
                       auto k = a_iter;
                       for(auto j = iter; j<iter_end; j++, k++)
                           *k=std::max(*k,*j);
                    }
                    return a;
               }
               else
               {
                   core::ltuple<long, N-1> shp;
                   std::copy(shape.begin(), shape.end() - 1, shp.begin());
                   core::ndarray<T,N-1> miny(shp, None);
                   std::transform(array.begin(), array.end(), miny.begin(), [=](core::ndarray<T,N-1> const& other) {return max(other, axis-1);});
                   return miny;
               }
           }


       PROXY(pythonic::numpy, min);
       PROXY(pythonic::numpy, max);

       template<class E>
           bool all(E&& expr) {
               long sz = expr.size();
               for(long i=0;i < sz ; ++i)
                   if( not expr.at(i) )
                       return false;
               return true;
           }

       template<class T>
            T all( core::ndarray<T,1> const& array, long axis)
            {
                if(axis!=0)
                    throw __builtin__::ValueError("axis out of bounds");
                return all(array);
            }

       template<class T, size_t N>
            typename core::ndarray<T,N>::value_type all( core::ndarray<T,N> const& array, long axis)
            {
                if(axis<0 || axis >=long(N))
                    throw __builtin__::ValueError("axis out of bounds");
                auto shape = array.shape;
                if(axis==0)
                {
                    core::ltuple<long, N-1> shp;
                    size_t size = 1;
                    for(auto i= shape.begin() + 1, j = shp.begin(); i<shape.end(); i++, j++)
                        size*=(*j = *i);
                    core::ndarray<T,N-1> a(shp, None);
                    auto a_iter = a.buffer;
                    std::copy(array.buffer, array.buffer + size, a_iter);
                    for(auto i = array.begin() + 1; i<array.end(); i++)
                    {
                        auto next_subarray = *i;  //we need this variable to keep this ndarray alive while iter is used
                        auto iter = next_subarray.buffer,
                             iter_end = next_subarray.buffer + next_subarray.size();
                        auto k = a_iter;
                        for(auto j = iter; j<iter_end; j++, k++)
                            *k=*k and *j;
                    }
                    return a;
                }
                else
                {
                    core::ltuple<long, N-1> shp;
                    std::copy(shape.begin(), shape.end() - 1, shp.begin());
                    core::ndarray<T,N-1> ally(shp, None);
                    std::transform(array.begin(), array.end(), ally.begin(), [=](core::ndarray<T,N-1> const& other) {return all(other, axis-1);});
                    return ally;
                }
            }

        PROXY(pythonic::numpy, all);

        template<class U, class V>
            bool allclose(U&& u, V&& v, double rtol=1e-5, double atol=1e-8) {
                long u_s = u.size(),
                     v_s = v.size();
                if( u_s == v_s ) {
                    for(long i=0;i < u_s; ++i) {
                        auto v_i = v.at(i);
                        if( std::abs(u.at(i)-v_i) > (atol + rtol * std::abs(v_i)))
                            return false;
                    }
                    return true;
                }
                return false;
            }

        PROXY(pythonic::numpy, allclose);

        template<class... Types>
            auto alltrue(Types&&... types) -> decltype(all(std::forward<Types>(types)...)) {
                return all(std::forward<Types>(types)...);
            }

        PROXY(pythonic::numpy, alltrue);

        template<class... Types>
            auto amax(Types&&... types) -> decltype(max(std::forward<Types>(types)...)) {
                return max(std::forward<Types>(types)...);
            }

        PROXY(pythonic::numpy, amax);

        template<class... Types>
            auto amin(Types&&... types) -> decltype(min(std::forward<Types>(types)...)) {
                return min(std::forward<Types>(types)...);
            }

        PROXY(pythonic::numpy, amin);

       template<class E>
           bool any(E&& expr) {
               long sz = expr.size();
               for(long i=0;i < sz ; ++i)
                   if( expr.at(i) )
                       return true;
               return false;
           }

       template<class T>
            T any( core::ndarray<T,1> const& array, long axis)
            {
                if(axis!=0)
                    throw __builtin__::ValueError("axis out of bounds");
                return any(array);
            }

       template<class T, size_t N>
            typename core::ndarray<T,N>::value_type any( core::ndarray<T,N> const& array, long axis)
            {
                if(axis<0 || axis >=long(N))
                    throw __builtin__::ValueError("axis out of bounds");
                auto shape = array.shape;
                if(axis==0)
                {
                    core::ltuple<long, N-1> shp;
                    size_t size = 1;
                    for(auto i= shape.begin() + 1, j = shp.begin(); i<shape.end(); i++, j++)
                        size*=(*j = *i);
                    core::ndarray<T,N-1> a(shp, None);
                    auto a_iter = a.buffer;
                    std::copy(array.buffer, array.buffer + size, a_iter);
                    for(auto i = array.begin() + 1; i<array.end(); i++)
                    {
                        auto next_subarray = *i;  //we need this variable to keep this ndarray alive while iter is used
                        auto iter = next_subarray.buffer,
                             iter_end = next_subarray.buffer + next_subarray.size();
                        auto k = a_iter;
                        for(auto j = iter; j<iter_end; j++, k++)
                            *k=*k or *j;
                    }
                    return a;
                }
                else
                {
                    core::ltuple<long, N-1> shp;
                    std::copy(shape.begin(), shape.end() - 1, shp.begin());
                    core::ndarray<T,N-1> ally(shp, None);
                    std::transform(array.begin(), array.end(), ally.begin(), [=](core::ndarray<T,N-1> const& other) {return any(other, axis-1);});
                    return ally;
                }
            }

        PROXY(pythonic::numpy, any);


        template<class T, unsigned long N, class... C>
            core::ndarray<T,N> _transpose(core::ndarray<T,N> const & a, long const l[N])
            {
                auto shape = a.shape;
                core::ltuple<long, N> shp;
                for(unsigned long i=0; i<N; i++)
                    shp[i] = shape[l[i]];

                core::ndarray<T,N> new_array(shp, None);

                std::array<long, N> new_strides;
                new_strides[N-1] = 1;
                std::transform(new_strides.rbegin(), new_strides.rend() -1, shp.rbegin(), new_strides.rbegin() + 1, std::multiplies<long>());

                std::array<long, N> old_strides;
                old_strides[N-1] = 1;
                std::transform(old_strides.rbegin(), old_strides.rend() -1, shape.rbegin(), old_strides.rbegin() + 1, std::multiplies<long>());

                auto iter = a.buffer,
                     iter_end = a.buffer + a.size();
                for(long i=0; iter!=iter_end; ++iter, ++i) {
                    long offset = 0;
                    for(unsigned long s=0; s<N; s++)
                        offset += ((i/old_strides[l[s]]) % shape[l[s]])*new_strides[s];
                    new_array.buffer[offset] = *iter;
                }

                return new_array;
            }

        template<class T, size_t N>
            core::ndarray<T,N> transpose(core::ndarray<T,N> const & a)
            {
                long t[N];
                for(unsigned long i = 0; i<N; i++)
                    t[N-1-i] = i;
                return _transpose(a, t);
            }
        template<class T, size_t N, size_t M>
            core::ndarray<T,N> transpose(core::ndarray<T,N> const & a, core::ltuple<long, M> const& t)
            {
                static_assert(N==M, "axes don't match array");

                long val = t[M-1];
                if(val>=long(N))
                    throw __builtin__::ValueError("invalid axis for this array");
                return _transpose(a, &t[0]);
            }

        NUMPY_EXPR_TO_NDARRAY0(transpose);
        PROXY(pythonic::numpy, transpose);
#if 0
        template<class... T, class type>
            core::ndarray<type,sizeof...(T)> build_cst_array_from_list(type cst, core::list<type> const& prev_l, type const& val, T const& ...l)
            {
                return core::ndarray<type, sizeof...(l)>({l...}, cst);
            }

        template<class... T, class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value + sizeof...(T)> build_cst_array_from_list(typename finalType<type>::Type cst, core::list<core::list<type> > const &prev_l, core::list<type> const& l, T const& ...s)
            {
                return build_cst_array_from_list(cst, l, l[0], s..., (size_t)l.size());
            }

        template<class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value> ones_like(core::list<type> const& l)
            {
                return build_cst_array_from_list(1, l, l[0], (size_t)l.size());
            }

        PROXY(pythonic::numpy, ones_like);

        template<class... T, class type>
            core::ndarray<type,sizeof...(T)> build_not_init_array_from_list(core::list<type> const& prev_l, type const& val, T ...l)
            {
                return core::ndarray<type, sizeof...(l)>({l...});
            }

        template<class... T, class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value + sizeof...(T)> build_not_init_array_from_list(core::list<core::list<type> > const &prev_l, core::list<type> const& l, T ...s)
            {
                return build_not_init_array_from_list(l, l[0], s..., (size_t)l.size());
            }

        template<class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value> zeros_like(core::list<type> const& l)
            {
                return build_cst_array_from_list(0, l, l[0], (size_t)l.size());
            }
        PROXY(pythonic::numpy, zeros_like);

        template<class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value> empty_like(core::list<type> const& l)
            {
                return build_not_init_array_from_list(l, l[0], (size_t)l.size());
            }

        PROXY(pythonic::numpy, empty_like);
#endif


#define NP_PROXY(name)\
        using nt2::name;\
        using pythonic::core::name;\
        PROXY(pythonic::numpy, name)
#define NP_PROXY_ALIAS(name, alias)\
        ALIAS(alias, name)\
        using pythonic::core::name;\
        PROXY(pythonic::numpy, name)
#define NP_PROXY_OP(name)\
        using pythonic::numpy_expr::ops::name;\
        using pythonic::core::name;\
        PROXY(pythonic::numpy, name)

        NP_PROXY(abs);

        NP_PROXY_ALIAS(absolute, nt2::abs);

        NP_PROXY_OP(add);

        NP_PROXY_ALIAS(angle_in_deg, pythonic::numpy_expr::ops::angle_in_deg);

        NP_PROXY_ALIAS(angle_in_rad, pythonic::numpy_expr::ops::angle_in_rad);

        template<class T>
            auto angle(T const& t, bool in_deg) -> decltype(typename core::numpy_expr_to_ndarray<T>::type(angle_in_rad(typename core::to_ndarray<T>::type(t)))) {
                if(in_deg)
                    return typename core::numpy_expr_to_ndarray<T>::type(angle_in_deg(typename core::to_ndarray<T>::type(t)));
                else
                    return typename core::numpy_expr_to_ndarray<T>::type(angle_in_rad(typename core::to_ndarray<T>::type(t)));
            }
        template<class T>
            auto angle(T const& t) -> typename std::enable_if<not core::is_numpy_expr<T>::value,decltype(angle(t,false))>::type {
                    return angle(t,false);
            }
        PROXY(pythonic::numpy, angle);

        template<class T, size_t N, class F>
            core::ndarray<
                typename std::remove_cv<
                    typename std::remove_reference<
                        decltype(
                                std::declval<T>()
                                +
                                std::declval<typename nested_container_value_type<F>::type>())
                        >::type
                    >::type,
                1> append(core::ndarray<T,N> const& nto, F const& data) {
                    typename core::numpy_expr_to_ndarray<F>::type ndata(data);
                    long nsize = nto.size() + ndata.size();
                    core::ndarray<
                        typename std::remove_cv<
                            typename std::remove_reference<
                                decltype(
                                        std::declval<T>()
                                        +
                                        std::declval<typename nested_container_value_type<F>::type>())
                                >::type
                            >::type,
                        1> out(core::make_tuple(nsize), None);
                    size_t i=0;
                    for(i=0;i<nto.size();i++)
                        out.at(i) = nto.at(i);
                    for(size_t j=0;j<ndata.size();j++)
                        out.at(i+j) = ndata.at(j);
                    return out;
                }
        template<class T, class F>
            core::ndarray<
                typename std::remove_cv<
                    typename std::remove_reference<
                        decltype(
                                std::declval<typename nested_container_value_type<core::list<T>>::type>()
                                +
                                std::declval<typename nested_container_value_type<F>::type>())
                        >::type
                    >::type,
                1> append(core::list<T> const& to, F const& data) {
                    return append(typename core::numpy_expr_to_ndarray<core::list<T>>::type(to), data);
                }

        PROXY(pythonic::numpy, append);

       template<class E>
           long argmin(E&& expr) {
               long sz = expr.size();
               if(not sz) 
                   throw __builtin__::ValueError("empty sequence");
               auto res = expr.at(0);
               long index = 0;
               for(long i = 1; i< sz ; ++i) {
                   auto e_i = expr.at(i);
                   if(e_i< res) {
                       res = e_i;
                       index = i;
                   }
               }
               return index;
           }
       template<class E>
           long argmax(E&& expr) {
               long sz = expr.size();
               if(not sz) 
                   throw __builtin__::ValueError("empty sequence");
               auto res = expr.at(0);
               long index = 0;
               for(long i = 1; i< sz ; ++i) {
                   auto e_i = expr.at(i);
                   if(e_i > res) {
                       res = e_i;
                       index = i;
                   }
               }
               return index;
           }
        PROXY(pythonic::numpy, argmax);

        PROXY(pythonic::numpy, argmin);

        template<class T, size_t N>
            core::ndarray<long, N> argsort(core::ndarray<T,N> const& a) {
                size_t last_axis = a.shape[N-1];
                size_t n = a.size();
                core::ndarray<long, N> indices(a.shape, None);
                for(long j=0, * iter_indices = indices.buffer, *end_indices = indices.buffer + n;
                        iter_indices != end_indices;
                        iter_indices += last_axis, j+=last_axis)
                {
                    // fill with the original indices
                    std::iota(iter_indices, iter_indices + last_axis, 0L);
                    // sort the index using the value from a
                    std::sort(iter_indices, iter_indices + last_axis,
                            [&a,j](long i1, long i2) {return a.at(j+i1) < a.at(j+i2);});
                }
                return indices;
            }

        PROXY(pythonic::numpy, argsort);

        template<class E>
            typename core::ndarray<long, 2>
            argwhere(E const& expr) {
                typedef typename core::ndarray<long, 2> out_type;
                constexpr long N = core::numpy_expr_to_ndarray<E>::N;
                long sz = expr.size();
                auto eshape = expr.shape;
                long *buffer = new long[N * sz]; // too much memory used
                long *buffer_iter = buffer;
                long real_sz = 0;
                for(long i=0; i< sz; ++i) {
                    if(expr.at(i)) {
                        ++real_sz;
                        long mult = 1;
                        for(long j=N-1; j>0; j--) {
                            buffer_iter[j] = (i/mult)%eshape[j];
                            mult*=eshape[j];
                        }
                        buffer_iter[0] = i/mult;
                        buffer_iter+=N;
                    }
                }
                long shape[2] = { real_sz, N };
                return out_type(buffer, shape, N*real_sz);
            }

        PROXY(pythonic::numpy, argwhere);

        template<class T, size_t N>
            core::ndarray<T,N> around(core::ndarray<T,N> const& a, long decimals=0) {
                return pythonic::core::rint(a * std::pow(T(10),decimals)) / std::pow(T(10), decimals);
            }
        template<class T>
            typename core::numpy_expr_to_ndarray<core::list<T>>::type around(core::list<T> const& l, long decimals=0) {
                return around(typename core::numpy_expr_to_ndarray<core::list<T>>::type(l), decimals);
            }

        PROXY(pythonic::numpy, around);

        template<class T, size_t N>
            core::string array2string(core::ndarray<T,N> const& a) {
                std::ostringstream oss;
                oss << a;
                return core::string(oss.str());
            }

        PROXY(pythonic::numpy, array2string);

        template<class U, class V>
            typename std::enable_if<has_shape<U>::value and has_shape<V>::value,bool>::type array_equal(U const& u, V const&v) {
                if(u.shape == v.shape) {
                    long n = u.size();
                    for(long i=0;i<n;i++)
                        if(u.at(i) != v.at(i))
                            return false;
                    return true;
                }
                return false;
            }
        template<class U, class V>
            typename std::enable_if<has_shape<V>::value,bool>::type array_equal(core::list<U> const& u, V const&v) {
                return array_equal(typename core::numpy_expr_to_ndarray<core::list<U>>::type(u), v);
            }
        template<class U, class V>
            typename std::enable_if<has_shape<U>::value,bool>::type array_equal(U const& u, core::list<V> const&v) {
                return array_equal(u, typename core::numpy_expr_to_ndarray<core::list<V>>::type(v));
            }
        template<class U, class V>
            bool array_equal(core::list<U> const& u, core::list<V> const&v) {
                return array_equal(typename core::numpy_expr_to_ndarray<core::list<U>>::type(u), typename core::numpy_expr_to_ndarray<core::list<V>>::type(v));
            }

        PROXY(pythonic::numpy, array_equal);

        NP_PROXY_ALIAS(arccos, nt2::acos);

        NP_PROXY_ALIAS(arccosh, nt2::acosh);

        NP_PROXY_ALIAS(arcsin, nt2::asin);

        NP_PROXY_ALIAS(arcsinh, nt2::asinh);

        NP_PROXY_ALIAS(arctan, nt2::atan);

        NP_PROXY_ALIAS(arctan2, nt2::atan2);

        NP_PROXY_ALIAS(arctanh, nt2::atanh);

        NP_PROXY_OP(bitwise_and);

        NP_PROXY_OP(bitwise_not);

        NP_PROXY_OP(bitwise_or);

        NP_PROXY_OP(bitwise_xor);

        NP_PROXY(ceil);

        // TODO
        // using pythonic::math::conj;
        // NP_PROXY(conj);
        //
        // using pythonic::math::conjugate;
        // NP_PROXY(conjugate);

        NP_PROXY(copysign);

        NP_PROXY(cos);

        NP_PROXY(cosh);

        NP_PROXY_ALIAS(deg2rad, nt2::inrad);

        NP_PROXY_ALIAS(degrees, nt2::indeg);

        NP_PROXY_OP(divide);

        NP_PROXY_ALIAS(empty_like, pythonic::numpy_expr::ops::empty_like);

        NP_PROXY_OP(equal);

        NP_PROXY(exp);

        NP_PROXY(expm1);

        NP_PROXY_ALIAS(fabs, nt2::abs);

        NP_PROXY(floor);

        NP_PROXY_ALIAS(floor_divide, nt2::divfloor);

        NP_PROXY_ALIAS(fmax, nt2::max);

        NP_PROXY_ALIAS(fmin, nt2::min);

        NP_PROXY_ALIAS(fmod, nt2::mod);

        // NP_PROXY(frexp); // TODO

        NP_PROXY_OP(greater);

        NP_PROXY_OP(greater_equal);

        NP_PROXY(hypot);

        NP_PROXY_ALIAS(invert, pythonic::numpy_expr::ops::bitwise_not); 

        NP_PROXY_ALIAS(isfinite, nt2::is_finite);

        NP_PROXY_ALIAS(isinf, nt2::is_inf);

        NP_PROXY_ALIAS(isnan, nt2::is_nan);

        NP_PROXY(ldexp);

        NP_PROXY_OP(left_shift);

        NP_PROXY_OP(less);

        NP_PROXY_OP(less_equal);

        NP_PROXY(log10);

        NP_PROXY(log1p);

        NP_PROXY(log2);

        NP_PROXY_ALIAS(logaddexp, pythonic::numpy_expr::ops::logaddexp);

        NP_PROXY_ALIAS(logaddexp2, pythonic::numpy_expr::ops::logaddexp2);

        NP_PROXY_OP(logical_and);

        NP_PROXY_OP(logical_not);

        NP_PROXY_OP(logical_or);

        NP_PROXY_OP(logical_xor);

        NP_PROXY_ALIAS(maximum, nt2::max);

        NP_PROXY_ALIAS(minimum, nt2::min);

        NP_PROXY(mod);

        NP_PROXY_OP(multiply);

        NP_PROXY_OP(negative);

        NP_PROXY(nextafter);

        NP_PROXY_OP(not_equal);

        NP_PROXY_ALIAS(ones_like, pythonic::numpy_expr::ops::ones_like);

        NP_PROXY_ALIAS(power, nt2::pow);

        NP_PROXY_ALIAS(rad2deg, nt2::indeg);

        NP_PROXY_ALIAS(radians, nt2::inrad);

        NP_PROXY_ALIAS(reciprocal, nt2::rec);

        NP_PROXY(remainder);

        NP_PROXY_OP(right_shift);

        NP_PROXY_ALIAS(rint, nt2::iround)

        NP_PROXY(sign);

        NP_PROXY_ALIAS(signbit, nt2::bitofsign)

        NP_PROXY(sin);

        NP_PROXY(sinh);

        NP_PROXY_ALIAS(spacing, nt2::eps)

        NP_PROXY(sqrt);

        NP_PROXY_ALIAS(square, pythonic::numpy_expr::ops::square);

        NP_PROXY_ALIAS(subtract, pythonic::numpy_expr::ops::subtract);

        NP_PROXY(tan);

        NP_PROXY(tanh);

        NP_PROXY_ALIAS(true_divide, pythonic::numpy_expr::ops::divide); 

        NP_PROXY(trunc);

        NP_PROXY_ALIAS(zeros_like, pythonic::numpy_expr::ops::zeros_like);

#undef NP_PROXY
#undef NAMED_OPERATOR
#undef NAMED_UOPERATOR
    }
}

#endif