//----------------------------------------------------------------------
//   Copyright 2020 Robert Bosch GmbH
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//----------------------------------------------------------------------

#ifndef UVM_OBJECT_DEFINES_H_
#define UVM_OBJECT_DEFINES_H_

// -------------------------------------------
// MACRO: UVM_OBJECT_UTILS
//
//! This macro is used to register the #uvm_object
//! to the factory and to add some dedicated
//! member functions
// -------------------------------------------

#define UVM_OBJECT_UTILS(...) \
   M_UVM_REPORT_METHODS_REDIRECTION(__VA_ARGS__) \
   M_UVM_OBJECT_REGISTRY_INTERNAL_PARAM(__VA_ARGS__) \
   M_UVM_OBJECT_GET_TYPE_NAME_FUNC_PARAM(__VA_ARGS__) \

#define UVM_OBJECT_PARAM_UTILS(...) \
   M_UVM_REPORT_METHODS_REDIRECTION(__VA_ARGS__) \
   M_UVM_OBJECT_REGISTRY_INTERNAL_PARAM(__VA_ARGS__) \
   M_UVM_OBJECT_GET_TYPE_NAME_FUNC_PARAM(__VA_ARGS__) \

// -------------------------------------------
// MACRO: M_UVM_REPORT_METHODS_REDIRECTION
//
// Implementation-defined macro  ----
// -------------------------------------------

#define M_UVM_REPORT_METHODS_REDIRECTION(...) \
    template<typename __PSI_TYPE>                                                                         \
    class IsSequenceItem {                                                                                \
        typedef char hasNoSequenceItem[1];                                                                \
        typedef char hasSequenceItem[2];                                                                  \
                                                                                                          \
        template<typename U>                                                                              \
        static hasSequenceItem& check(typename U::uvm_sequence_item*);                                    \
                                                                                                          \
        template<typename>                                                                                \
        static hasNoSequenceItem& check(...);                                                             \
                                                                                                          \
    public:                                                                                               \
        static const bool result = (sizeof(check<__PSI_TYPE>(0)) == sizeof(hasSequenceItem));             \
    };                                                                                                    \
                                                                                                          \
    /* uvm_report_enabled */                                                                              \
    template <typename __PSI_TYPE = __VA_ARGS__>                                                          \
    typename std::enable_if<IsSequenceItem<__PSI_TYPE>::result,bool>::type uvm_report_enabled(            \
            int verbosity,                                                                                \
            uvm::uvm_severity severity = uvm::UVM_INFO,                                                   \
            std::string id = "" ) const {                                                                 \
       return ((typename __PSI_TYPE::uvm_sequence_item*)this)->uvm_report_enabled(verbosity,severity,id); \
    }                                                                                                     \
                                                                                                          \
    template <typename __PSI_TYPE = __VA_ARGS__>                                                          \
    typename std::enable_if<!IsSequenceItem<__PSI_TYPE>::result,bool>::type uvm_report_enabled(           \
            int verbosity,                                                                                \
            uvm::uvm_severity severity = uvm::UVM_INFO,                                                   \
            std::string id = "" ) const {                                                                 \
       return uvm::uvm_report_enabled(verbosity,severity,id);                                             \
    }                                                                                                     \
                                                                                                          \
    /* uvm_report */                                                                                      \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<IsSequenceItem<__PSI_TYPE>::result>::type uvm_report(                         \
            uvm::uvm_severity severity,                                                                   \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_MEDIUM,                                                              \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        ((typename __PSI_TYPE::uvm_sequence_item*)this)->uvm_report_info(                                 \
                id,                                                                                       \
                message,                                                                                  \
                verbosity,                                                                                \
                filename,                                                                                 \
                line,                                                                                     \
                context_name,                                                                             \
                report_enabled_checked                                                                    \
                );                                                                                        \
    }                                                                                                     \
                                                                                                          \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<!IsSequenceItem<__PSI_TYPE>::result>::type uvm_report(                        \
            uvm::uvm_severity severity,                                                                   \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_NONE,                                                                \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        uvm::uvm_report_info(id,message,verbosity,filename,line,context_name,report_enabled_checked);     \
    }                                                                                                     \
                                                                                                          \
    /* uvm_report_info */                                                                                 \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_info(                    \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_MEDIUM,                                                              \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        ((typename __PSI_TYPE::uvm_sequence_item*)this)->uvm_report_info(                                 \
                id,                                                                                       \
                message,                                                                                  \
                verbosity,                                                                                \
                filename,                                                                                 \
                line,                                                                                     \
                context_name,                                                                             \
                report_enabled_checked                                                                    \
                );                                                                                        \
    }                                                                                                     \
                                                                                                          \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<!IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_info(                   \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_NONE,                                                                \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        uvm::uvm_report_info(id,message,verbosity,filename,line,context_name,report_enabled_checked);     \
    }                                                                                                     \
                                                                                                          \
    /* uvm_report_warning */                                                                              \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_warning(                 \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_MEDIUM,                                                              \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        ((typename __PSI_TYPE::uvm_sequence_item*)this)->uvm_report_warning(                              \
                id,                                                                                       \
                message,                                                                                  \
                verbosity,                                                                                \
                filename,                                                                                 \
                line,                                                                                     \
                context_name,                                                                             \
                report_enabled_checked                                                                    \
                );                                                                                        \
    }                                                                                                     \
                                                                                                          \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<!IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_warning(                \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_NONE,                                                                \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        uvm::uvm_report_warning(id,message,verbosity,filename,line,context_name,report_enabled_checked);  \
    }                                                                                                     \
                                                                                                          \
    /* uvm_report_error */                                                                                \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_error(                   \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_LOW,                                                                 \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        ((typename __PSI_TYPE::uvm_sequence_item*)this)->uvm_report_error(                                \
                id,                                                                                       \
                message,                                                                                  \
                verbosity,                                                                                \
                filename,                                                                                 \
                line,                                                                                     \
                context_name,                                                                             \
                report_enabled_checked                                                                    \
                );                                                                                        \
    }                                                                                                     \
                                                                                                          \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<!IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_error(                  \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_NONE,                                                                \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        uvm::uvm_report_error(id,message,verbosity,filename,line,context_name,report_enabled_checked);    \
    }                                                                                                     \
                                                                                                          \
    /* uvm_report_fatal */                                                                                \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_fatal(                   \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_NONE,                                                                \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        ((typename __PSI_TYPE::uvm_sequence_item*)this)->uvm_report_fatal(                                \
                id,                                                                                       \
                message,                                                                                  \
                verbosity,                                                                                \
                filename,                                                                                 \
                line,                                                                                     \
                context_name,                                                                             \
                report_enabled_checked                                                                    \
                );                                                                                        \
    }                                                                                                     \
                                                                                                          \
    template<typename __PSI_TYPE = __VA_ARGS__>                                                           \
    typename std::enable_if<!IsSequenceItem<__PSI_TYPE>::result>::type uvm_report_fatal(                  \
            const std::string& id,                                                                        \
            const std::string& message,                                                                   \
            int verbosity = uvm::UVM_NONE,                                                                \
            const std::string& filename = "",                                                             \
            int line = 0,                                                                                 \
            const std::string& context_name = "",                                                         \
            bool report_enabled_checked = false ) const {                                                 \
        uvm::uvm_report_fatal(id,message,verbosity,filename,line,context_name,report_enabled_checked);    \
    }                                                                                                     \


// -------------------------------------------
// MACRO: M_UVM_OBJECT_REGISTRY_INTERNAL
//
// Implementation-defined macro  ----
// -------------------------------------------

#define M_UVM_OBJECT_REGISTRY_INTERNAL_PARAM(...) \
  static const std::string m_register_type_name() { \
    return std::string(#__VA_ARGS__); } \
  typedef ::uvm::uvm_object_registry<__VA_ARGS__ > type_id; \
  static type_id* get_type() { return type_id::get(); } \

// -------------------------------------------
// MACRO: M_UVM_GET_TYPE_NAME_FUNC
//
// Implementation-defined macro
// -------------------------------------------

#define M_UVM_OBJECT_GET_TYPE_NAME_FUNC_PARAM(...) \
  virtual const std::string get_type_name() const { \
     return #__VA_ARGS__; \
  } \
  virtual const char* kind() const { \
    return #__VA_ARGS__; \
  } \


#endif /* UVM_OBJECT_DEFINES_H_ */
