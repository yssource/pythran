#ifndef PYTHONIC_INCLUDE_BUILTIN_PYTHRAN_IS_NONE_HPP
#define PYTHONIC_INCLUDE_BUILTIN_PYTHRAN_IS_NONE_HPP

#include "pythonic/include/utils/functor.hpp"
#include "pythonic/include/types/NoneType.hpp"

PYTHONIC_NS_BEGIN

namespace types
{

  struct false_type;

  struct true_type {
    operator bool() const
    {
      return true;
    }
    false_type operator!() const;
    true_type operator&(true_type) const;
    false_type operator&(false_type) const;
    true_type operator|(true_type) const;
    true_type operator|(false_type) const;
    true_type operator==(true_type) const;
    false_type operator==(false_type) const;
  };

  struct false_type {
    operator bool() const
    {
      return false;
    }
    true_type operator!() const
    {
      return {};
    }
    false_type operator&(true_type)
    {
      return {};
    }
    false_type operator&(false_type)
    {
      return {};
    }
    true_type operator|(true_type)
    {
      return {};
    }
    false_type operator|(false_type)
    {
      return {};
    }
    false_type operator==(true_type)
    {
      return {};
    }
    true_type operator==(false_type)
    {
      return {};
    }
  };

  false_type true_type::operator!() const
  {
    return {};
  }
  true_type true_type::operator&(true_type) const
  {
    return {};
  }
  false_type true_type::operator&(false_type) const
  {
    return {};
  }
  true_type true_type::operator|(true_type) const
  {
    return {};
  }
  true_type true_type::operator|(false_type) const
  {
    return {};
  }
  true_type true_type::operator==(true_type) const
  {
    return {};
  }
  false_type true_type::operator==(false_type) const
  {
    return {};
  }
}

namespace __builtin__
{

  namespace pythran
  {
    template <class T>
    types::false_type is_none(T const &)
    {
      return {};
    };

    template <class T>
    bool is_none(types::none<T> const &n)
    {
      return n.is_none;
    };

    types::true_type is_none(types::none_type const &)
    {
      return {};
    };

    DEFINE_FUNCTOR(pythonic::__builtin__::pythran, is_none);
  }
}
template <>
struct to_python<types::true_type> : to_python<bool> {
};
template <>
struct to_python<types::false_type> : to_python<bool> {
};
PYTHONIC_NS_END

#endif
