#ifndef PYTHONIC_INCLUDE_NUMPY_LINALG_MATRIX_POWER_HPP
#define PYTHONIC_INCLUDE_NUMPY_LINALG_MATRIX_POWER_HPP

#include "pythonic/include/numpy/asarray.hpp"

namespace pythonic
{
  namespace numpy
  {
    namespace linalg
    {
      template <class E>
      auto matrix_power(E const &expr, int n)
          -> decltype(numpy::functor::asarray{}(expr));

      DECLARE_FUNCTOR(pythonic::numpy::linalg, matrix_power);
    }
  }
}

#endif
