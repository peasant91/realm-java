/*
 * Copyright 2017 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef REALM_JNI_IMPL_JAVA_ACCESSOR_HPP
#define REALM_JNI_IMPL_JAVA_ACCESSOR_HPP

#include <jni.h>

#include <vector>

#include <realm/binary_data.hpp>

#include "java_exception_def.hpp"
#include "jni_util/java_exception_thrower.hpp"

namespace realm {
namespace _impl {

template <typename, typename>
class JArrayAccessor;
typedef JArrayAccessor<jbyteArray, jbyte> JByteArrayAccessor;
typedef JArrayAccessor<jbooleanArray, jboolean> JBooleanArrayAccessor;
typedef JArrayAccessor<jlongArray, jlong> JLongArrayAccessor;

template <typename ArrayType, typename ElementType>
class JArrayAccessor {
public:
    JArrayAccessor(JNIEnv* env, ArrayType jarray);
    ~JArrayAccessor();

    inline jsize size() const noexcept
    {
        return m_size;
    }

    inline ElementType* data() const noexcept
    {
        return m_data_ptr;
    }

    inline const ElementType& operator[](const int index) const noexcept
    {
        return m_data_ptr[index];
    }

    template <typename T>
    T transform();

private:
    JNIEnv* m_env;
    ArrayType m_jarray;
    jsize m_size;
    ElementType* m_data_ptr;
    jint m_releaseMode;

    inline void check_init()
    {
        if (m_jarray != nullptr && m_data_ptr == nullptr) {
            THROW_JAVA_EXCEPTION(
                m_env, JavaExceptionDef::IllegalArgument,
                util::format("GetXxxArrayElements failed on %1.", reinterpret_cast<int64_t>(m_jarray)));
        }
    }
};

// Accessor for jbyteArray
template <>
inline JArrayAccessor<jbyteArray, jbyte>::JArrayAccessor(JNIEnv* env, jbyteArray jarray)
    : m_env(env)
    , m_jarray(jarray)
    , m_size(jarray ? env->GetArrayLength(jarray) : 0)
    , m_data_ptr(jarray ? env->GetByteArrayElements(jarray, nullptr) : nullptr)
    , m_releaseMode(JNI_ABORT)
{
    check_init();
}

template <>
inline JArrayAccessor<jbyteArray, jbyte>::~JArrayAccessor()
{
    if (m_jarray) {
        m_env->ReleaseByteArrayElements(m_jarray, m_data_ptr, m_releaseMode);
    }
}

template <>
template <>
inline BinaryData JArrayAccessor<jbyteArray, jbyte>::transform<BinaryData>()
{
    return m_jarray ? realm::BinaryData(reinterpret_cast<const char*>(m_data_ptr), m_size) : realm::BinaryData();
}

template <>
template <>
inline std::vector<char> JArrayAccessor<jbyteArray, jbyte>::transform<std::vector<char>>()
{
    if (m_jarray == nullptr) {
        return {};
    }

    std::vector<char> v(m_size);
    std::copy_n(m_data_ptr, v.size(), v.begin());
    return v;
}

// Accessor for jbooleanArray
template <>
inline JArrayAccessor<jbooleanArray, jboolean>::JArrayAccessor(JNIEnv* env, jbooleanArray jarray)
    : m_env(env)
    , m_jarray(jarray)
    , m_size(jarray ? env->GetArrayLength(jarray) : 0)
    , m_data_ptr(jarray ? env->GetBooleanArrayElements(jarray, nullptr) : nullptr)
    , m_releaseMode(JNI_ABORT)
{
    check_init();
}

template <>
inline JArrayAccessor<jbooleanArray, jboolean>::~JArrayAccessor()
{
    if (m_jarray) {
        m_env->ReleaseBooleanArrayElements(m_jarray, m_data_ptr, m_releaseMode);
    }
}

// Accessor for jlongArray
template <>
inline JArrayAccessor<jlongArray, jlong>::JArrayAccessor(JNIEnv* env, jlongArray jarray)
    : m_env(env)
    , m_jarray(jarray)
    , m_size(jarray ? env->GetArrayLength(jarray) : 0)
    , m_data_ptr(jarray ? env->GetLongArrayElements(jarray, nullptr) : nullptr)
    , m_releaseMode(JNI_ABORT)
{
    check_init();
}

template <>
inline JArrayAccessor<jlongArray, jlong>::~JArrayAccessor()
{
    if (m_jarray) {
        m_env->ReleaseLongArrayElements(m_jarray, m_data_ptr, m_releaseMode);
    }
}

} // namespace realm
} // namespace _impl

#endif // REALM_JNI_IMPL_JAVA_ACCESSOR_HPP
