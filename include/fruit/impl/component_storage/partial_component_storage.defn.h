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

#ifndef FRUIT_PARTIAL_COMPONENT_STORAGE_DEFN_H
#define FRUIT_PARTIAL_COMPONENT_STORAGE_DEFN_H

#include <fruit/impl/component_storage/partial_component_storage.h>

#include <fruit/impl/util/type_info.h>
#include <fruit/impl/bindings.h>
#include <fruit/impl/component_storage/component_storage.h>
#include <fruit/impl/injector/injector_storage.h>
#include <fruit/impl/util/call_with_tuple.h>
#include <utility>

namespace fruit {
namespace impl {

template <>
class PartialComponentStorage<> {
public:
  void addBindings(ComponentStorage& storage) const {
    (void)storage;
  }
};


template <typename I, typename C, typename... PreviousBindings>
class PartialComponentStorage<Bind<I, C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {
  }
  
  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
  }
};

template <typename Signature, typename... PreviousBindings>
class PartialComponentStorage<RegisterConstructor<Signature>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
  }
};

template <typename C, typename C1, typename... PreviousBindings>
class PartialComponentStorage<BindInstance<C, C1>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  C &instance;

public:
  PartialComponentStorage(
      PartialComponentStorage<PreviousBindings...>& previous_storage,
      C& instance)
      : previous_storage(previous_storage), instance(instance) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
    storage.addEntry(InjectorStorage::createComponentStorageEntryForBindInstance<C, C>(instance));
  }
};

template <typename C, typename Annotation, typename C1, typename... PreviousBindings>
class PartialComponentStorage<BindInstance<fruit::Annotated<Annotation, C>, C1>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  C &instance;

public:
  PartialComponentStorage(
      PartialComponentStorage<PreviousBindings...>& previous_storage,
      C& instance)
      : previous_storage(previous_storage), instance(instance) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
    storage.addEntry(
        InjectorStorage::createComponentStorageEntryForBindInstance<fruit::Annotated<Annotation, C>, C>(instance));
  }
};

template <typename... Params, typename... PreviousBindings>
class PartialComponentStorage<RegisterProvider<Params...>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
  }
};

template <typename C, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceMultibinding<C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, C& instance)
      : previous_storage(previous_storage), instance(instance) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
    storage.addEntry(
        InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<C>());
    storage.addEntry(
        InjectorStorage::createComponentStorageEntryForInstanceMultibinding<C, C>(instance));
  }
};

template <typename C, typename Annotation, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceMultibinding<fruit::Annotated<Annotation, C>>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, C& instance)
      : previous_storage(previous_storage), instance(instance) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
    storage.addEntry(
        InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<fruit::Annotated<Annotation, C>>());
    storage.addEntry(
        InjectorStorage::createComponentStorageEntryForInstanceMultibinding<fruit::Annotated<Annotation, C>, C>(instance));
  }
};

template <typename C, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceVectorMultibindings<C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  std::vector<C>& instances;

public:
  PartialComponentStorage(
      PartialComponentStorage<PreviousBindings...>& previous_storage,
      std::vector<C>& instances)
      : previous_storage(previous_storage), instances(instances) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
    for (C& instance : instances) {
      // TODO: consider optimizing this so that we need just 1 MULTIBINDING_VECTOR_CREATOR entry (removing the
      // assumption that each multibinding entry is always preceded by that).
      storage.addEntry(
          InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<C>());
      storage.addEntry(InjectorStorage::createComponentStorageEntryForInstanceMultibinding<C, C>(instance));
    }
  }
};

template <typename C, typename Annotation, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceVectorMultibindings<fruit::Annotated<Annotation, C>>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  std::vector<C>& instances;

public:
  PartialComponentStorage(
      PartialComponentStorage<PreviousBindings...>& previous_storage,
      std::vector<C>& instances)
      : previous_storage(previous_storage), instances(instances) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
    for (C& instance : instances) {
      // TODO: consider optimizing this so that we need just 1 MULTIBINDING_VECTOR_CREATOR entry (removing the
      // assumption that each multibinding entry is always preceded by that).
      storage.addEntry(
          InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<fruit::Annotated<Annotation, C>>());
      storage.addEntry(
          InjectorStorage::createComponentStorageEntryForInstanceMultibinding<fruit::Annotated<Annotation, C>, C>(instance));
    }
  }
};

template <typename I, typename C, typename... PreviousBindings>
class PartialComponentStorage<AddMultibinding<I, C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
  }
};

template <typename... Params, typename... PreviousBindings>
class PartialComponentStorage<AddMultibindingProvider<Params...>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {
  }

  void addBindings(ComponentStorage& storage) {
    previous_storage.addBindings(storage);
  }
};

template <typename DecoratedSignature, typename Lambda, typename... PreviousBindings>
class PartialComponentStorage<RegisterFactory<DecoratedSignature, Lambda>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {
  }

  void addBindings(ComponentStorage& storage) const {
    previous_storage.addBindings(storage);
  }
};

template <typename OtherComponent, typename... PreviousBindings>
class PartialComponentStorage<OldStyleInstallComponent<OtherComponent>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  ComponentStorage installed_component_storage;

public:
  PartialComponentStorage(
      PartialComponentStorage<PreviousBindings...>& previous_storage,
      ComponentStorage&& installed_component_storage)
      : previous_storage(previous_storage), installed_component_storage(installed_component_storage) {
  }

  void addBindings(ComponentStorage& storage) {
    previous_storage.addBindings(storage);
    storage.addAll(std::move(installed_component_storage));
  }
};

template <typename OtherComponent, typename... PreviousBindings>
class PartialComponentStorage<InstallComponent<OtherComponent>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  OtherComponent(*fun)();

public:
  PartialComponentStorage(
      PartialComponentStorage<PreviousBindings...>& previous_storage,
      OtherComponent(*fun1)())
      : previous_storage(previous_storage),
        fun(fun1) {
  }

  void addBindings(ComponentStorage& storage) {
    previous_storage.addBindings(storage);
    storage.addEntry(ComponentStorageEntry::LazyComponentWithNoArgs::create(fun));
  }
};

template <typename OtherComponent, typename... Args, typename... PreviousBindings>
class PartialComponentStorage<InstallComponent<OtherComponent, Args...>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...> &previous_storage;
  OtherComponent(*fun)(Args...);
  std::tuple<Args...> args_tuple;

public:
  PartialComponentStorage(
      PartialComponentStorage<PreviousBindings...>& previous_storage,
      OtherComponent(*fun1)(Args...),
      typename std::remove_const<typename std::remove_reference<Args>::type>::type... args)
      : previous_storage(previous_storage),
        fun(fun1),
        args_tuple{std::move(args)...} {
  }

  void addBindings(ComponentStorage& storage) {
    previous_storage.addBindings(storage);
    storage.addEntry(ComponentStorageEntry::LazyComponentWithArgs::create(fun, std::move(args_tuple)));
  }
};


} // namespace impl
} // namespace fruit

#endif // FRUIT_PARTIAL_COMPONENT_STORAGE_DEFN_H