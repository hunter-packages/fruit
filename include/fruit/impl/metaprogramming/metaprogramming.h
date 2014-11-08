/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRUIT_METAPROGRAMMING_H
#define FRUIT_METAPROGRAMMING_H

#include "basics.h"
#include "list.h"

#include "../fruit_assert.h"
#include "../fruit_internal_forward_decls.h"

namespace fruit {
namespace impl {
  
template <int...>
struct IntList {};

#ifdef FRUIT_EXTRA_DEBUG

template <typename T>
struct DebugTypeHelper {
  static_assert(sizeof(T*)*0 != 0, "");
  using type = T;
};

template <typename T>
using DebugType = typename DebugTypeHelper<T>::type;

#endif

struct IsConstructibleWithList {
  template <typename C, typename L>
  struct apply;

  template <typename C, typename... Types>
  struct apply<C, List<Types...>> : public std::is_constructible<C, Types...> {};
};

struct SignatureType {
  template <typename Signature>
  struct apply;

  template <typename T, typename... Types>
  struct apply<T(Types...)> {
    using type = T;
  };
};

struct SignatureArgs {
  template <typename Signature>
  struct apply;

  template <typename T, typename... Types>
  struct apply<T(Types...)> {
    using type = List<Types...>;
  };
};

struct ConstructSignature {
  template <typename T, typename L>
  struct apply;

  template <typename T, typename... Types>
  struct apply<T, List<Types...>> {
    using type = T(Types...);
  };
};

struct AddPointerToList {
  template <typename L>
  struct apply;

  template <typename... Ts>
  struct apply<List<Ts...>> {
    using type = List<Ts*...>;
  };
};

template<int n, int... ns>
struct GenerateIntSequenceHelper : public GenerateIntSequenceHelper<n-1, n-1, ns...> {};

template<int... ns>
struct GenerateIntSequenceHelper<0, ns...> {
  using type = IntList<ns...>;
};

template <int n>
using GenerateIntSequence = typename GenerateIntSequenceHelper<n>::type;

struct GetNthTypeHelper {
  template <int n, typename... Ts>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<0, T, Ts...> {
    using type = T;
  };

  template <int n, typename T, typename... Ts>
  struct apply<n, T, Ts...> : public apply<n-1, Ts...> {};
};

struct GetNthTypeImpl {
  template <int n, typename L>
  struct apply;

  template <int n, typename... Ts>
  struct apply<n, List<Ts...>> : public GetNthTypeHelper::template apply<n, Ts...>{
  };
};

template <int n, typename L>
using GetNthType = typename GetNthTypeImpl::template apply<n, L>::type;

struct FunctorResultHelper {
  template <typename MethodSignature>
  struct apply {};

  template <typename Result, typename Functor, typename... Args>
  struct apply<Result(Functor::*)(Args...)> {
    using type = Result;
  };
};

struct FunctorResult {
  template <typename F>
  struct apply {
    using type = Apply<FunctorResultHelper, decltype(&F::operator())>;
  };
};

struct FunctionSignatureHelper {
  template <typename LambdaMethod>
  struct apply {};

  template <typename Result, typename LambdaObject, typename... Args>
  struct apply<Result(LambdaObject::*)(Args...) const> {
    using type = Result(Args...);
  };
};

// Function is either a plain function type of the form T(*)(Args...) or a lambda.
struct FunctionSignature {
  template <typename Function>
  struct apply {
    using type = Apply<FunctionSignatureHelper, decltype(&Function::operator())>;
    FruitDelegateCheck(fruit::impl::FunctorUsedAsProvider<type, Function>);
  };

  template <typename Result, typename... Args>
  struct apply<Result(Args...)> {
    using type = Result(Args...);
  };

  template <typename Result, typename... Args>
  struct apply<Result(*)(Args...)> {
    using type = Result(Args...);
  };
};

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_H