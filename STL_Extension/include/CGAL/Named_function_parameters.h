// Copyright (c) 2019  GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot

#ifndef CGAL_NAMED_FUNCTION_PARAMETERS_H
#define CGAL_NAMED_FUNCTION_PARAMETERS_H

#ifndef CGAL_NO_STATIC_ASSERTION_TESTS
#include <CGAL/basic.h>
#endif

#include <CGAL/tags.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/if.hpp>

#include <type_traits>
#include <utility>

#define CGAL_NP_TEMPLATE_PARAMETERS NP_T=bool, typename NP_Tag=CGAL::internal_np::all_default_t, typename NP_Base=CGAL::internal_np::No_property
#define CGAL_NP_TEMPLATE_PARAMETERS_NO_DEFAULT NP_T, typename NP_Tag, typename NP_Base
#define CGAL_NP_CLASS CGAL::Named_function_parameters<NP_T,NP_Tag,NP_Base>

#define CGAL_NP_TEMPLATE_PARAMETERS_1 NP_T1=bool, typename NP_Tag1=CGAL::internal_np::all_default_t, typename NP_Base1=CGAL::internal_np::No_property
#define CGAL_NP_CLASS_1 CGAL::Named_function_parameters<NP_T1,NP_Tag1,NP_Base1>
#define CGAL_NP_TEMPLATE_PARAMETERS_2 NP_T2=bool, typename NP_Tag2=CGAL::internal_np::all_default_t, typename NP_Base2=CGAL::internal_np::No_property
#define CGAL_NP_CLASS_2 CGAL::Named_function_parameters<NP_T2,NP_Tag2,NP_Base2>

namespace CGAL {
namespace internal_np{

struct No_property {};
struct Param_not_found {};

enum all_default_t { all_default };

// define enum types and values for new named parameters
#define CGAL_add_named_parameter(X, Y, Z)            \
  enum X { Y };
#include <CGAL/STL_Extension/internal/parameters_interface.h>
#undef CGAL_add_named_parameter

template <typename T, typename Tag, typename Base>
struct Named_params_impl : Base
{
  typename std::conditional<std::is_copy_constructible<T>::value,
                            T, std::reference_wrapper<const T> >::type v; // copy of the parameter if copyable
  Named_params_impl(const T& v, const Base& b)
    : Base(b)
    , v(v)
  {}
};

// partial specialization for base class of the recursive nesting
template <typename T, typename Tag>
struct Named_params_impl<T, Tag, No_property>
{
  typename std::conditional<std::is_copy_constructible<T>::value,
                            T, std::reference_wrapper<const T> >::type v; // copy of the parameter if copyable
  Named_params_impl(const T& v)
    : v(v)
  {}
};

// Helper class to get the type of a named parameter pack given a query tag
template <typename NP, typename Query_tag>
struct Get_param;

template< typename T, typename Tag, typename Query_tag>
struct Get_param< Named_params_impl<T, Tag, No_property>, Query_tag >
{
  typedef Param_not_found type;
  typedef Param_not_found reference;
};

template< typename T, typename Tag, typename Base>
struct Get_param< Named_params_impl<T, Tag, Base>, Tag >
{
  typedef typename std::conditional<std::is_copy_constructible<T>::value,
                                    T, std::reference_wrapper<const T> >::type type;
  typedef typename std::conditional<std::is_copy_constructible<T>::value,
                                    T, const T&>::type reference;
};

template< typename T, typename Tag>
struct Get_param< Named_params_impl<T, Tag, No_property>, Tag >
{
  typedef typename std::conditional<std::is_copy_constructible<T>::value,
                                    T, std::reference_wrapper<const T> >::type type;
  typedef typename std::conditional<std::is_copy_constructible<T>::value,
                                    T, const T&>::type reference;
};

template< typename T, typename Tag, typename Base>
struct Get_param< Named_params_impl<std::reference_wrapper<T>, Tag, Base>, Tag >
{
  typedef std::reference_wrapper<T> type;
  typedef T& reference;
};

template< typename T, typename Tag>
struct Get_param< Named_params_impl<std::reference_wrapper<T>, Tag, No_property>, Tag >
{
  typedef std::reference_wrapper<T> type;
  typedef T& reference;
};


template< typename T, typename Tag, typename Base, typename Query_tag>
struct Get_param< Named_params_impl<T,Tag,Base>, Query_tag>
{
  typedef typename Get_param<typename Base::base, Query_tag>::type type;
  typedef typename Get_param<typename Base::base, Query_tag>::reference reference;
};

// helper to choose the default
template <typename Query_tag, typename NP, typename D>
struct Lookup_named_param_def
{
  typedef typename internal_np::Get_param<typename NP::base, Query_tag>::type NP_type;
  typedef typename internal_np::Get_param<typename NP::base, Query_tag>::reference NP_reference;

  typedef typename boost::mpl::if_<
    boost::is_same<NP_type, internal_np::Param_not_found>,
    D, NP_type>::type
  type;

  typedef typename boost::mpl::if_<
    boost::is_same<NP_reference, internal_np::Param_not_found>,
    D&, NP_reference>::type
  reference;
};

// helper function to extract the value from a named parameter pack given a query tag
template <typename T, typename Tag, typename Base>
typename std::conditional<std::is_copy_constructible<T>::value,
                          T, std::reference_wrapper<const T> >::type
get_parameter_impl(const Named_params_impl<T, Tag, Base>& np, Tag)
{
  return np.v;
}

template< typename T, typename Tag, typename Query_tag>
Param_not_found get_parameter_impl(const Named_params_impl<T, Tag, No_property>&, Query_tag)
{
  return Param_not_found();
}

template< typename T, typename Tag>
typename std::conditional<std::is_copy_constructible<T>::value,
                          T, std::reference_wrapper<const T> >::type
get_parameter_impl(const Named_params_impl<T, Tag, No_property>& np, Tag)
{
  return np.v;
}

template <typename T, typename Tag, typename Base, typename Query_tag>
typename Get_param<Named_params_impl<T, Tag, Base>, Query_tag>::type
get_parameter_impl(const Named_params_impl<T, Tag, Base>& np, Query_tag tag)
{
#ifndef CGAL_NO_STATIC_ASSERTION_TEST
  CGAL_static_assertion( (!boost::is_same<Query_tag, Tag>::value) );
#endif
  return get_parameter_impl(static_cast<const typename Base::base&>(np), tag);
}


// helper for getting references
template <class T>
const T& get_reference(const T& t)
{
  return t;
}

template <class T>
T& get_reference(const std::reference_wrapper<T>& r)
{
  return r.get();
}

// helper function to extract the reference from a named parameter pack given a query tag
template <typename T, typename Tag, typename Base>
typename std::conditional<std::is_copy_constructible<T>::value,
                          T, const T& >::type
get_parameter_reference_impl(const Named_params_impl<T, Tag, Base>& np, Tag)
{
  return get_reference(np.v);
}

template< typename T, typename Tag, typename Query_tag>
Param_not_found
get_parameter_reference_impl(const Named_params_impl<T, Tag, No_property>&, Query_tag)
{
  return Param_not_found();
}

template< typename T, typename Tag>
typename std::conditional<std::is_copy_constructible<T>::value,
                          T, const T& >::type
get_parameter_reference_impl(const Named_params_impl<T, Tag, No_property>& np, Tag)
{
  return get_reference(np.v);
}

template <typename T, typename Tag, typename Base>
T&
get_parameter_reference_impl(const Named_params_impl<std::reference_wrapper<T>, Tag, Base>& np, Tag)
{
  return np.v.get();
}

template< typename T, typename Tag>
T&
get_parameter_reference_impl(const Named_params_impl<std::reference_wrapper<T>, Tag, No_property>& np, Tag)
{
  return np.v.get();
}

template <typename T, typename Tag, typename Base, typename Query_tag>
typename Get_param<Named_params_impl<T, Tag, Base>, Query_tag>::reference
get_parameter_reference_impl(const Named_params_impl<T, Tag, Base>& np, Query_tag tag)
{
  CGAL_static_assertion( (!boost::is_same<Query_tag, Tag>::value) );
  return get_parameter_reference_impl(static_cast<const typename Base::base&>(np), tag);
}


} // end of internal_np namespace


template <typename T, typename Tag, typename Base = internal_np::No_property>
struct Named_function_parameters
  : internal_np::Named_params_impl<T, Tag, Base>
{
  typedef internal_np::Named_params_impl<T, Tag, Base> base;
  typedef Named_function_parameters<T, Tag, Base> self;

  Named_function_parameters() : base(T()) {}
  Named_function_parameters(const T& v) : base(v) {}
  Named_function_parameters(const T& v, const Base& b) : base(v, b) {}

// create the functions for new named parameters and the one imported boost
// used to concatenate several parameters
#define CGAL_add_named_parameter(X, Y, Z)                          \
  template<typename K>                                           \
  Named_function_parameters<K, internal_np::X, self>                  \
  Z(const K& k) const                                            \
  {                                                              \
    typedef Named_function_parameters<K, internal_np::X, self> Params;\
    return Params(k, *this);                                     \
  }
#include <CGAL/STL_Extension/internal/parameters_interface.h>
#undef CGAL_add_named_parameter

  template <typename OT, typename OTag>
  Named_function_parameters<OT, OTag, self>
  combine(const Named_function_parameters<OT,OTag>& np) const
  {
    return Named_function_parameters<OT, OTag, self>(np.v,*this);
  }

  template <typename OT, typename OTag, typename ... NPS>
  auto
  combine(const Named_function_parameters<OT,OTag>& np, const NPS& ... nps) const
  {
    return Named_function_parameters<OT, OTag, self>(np.v,*this).combine(nps...);
  }
};

namespace parameters {

typedef Named_function_parameters<bool, internal_np::all_default_t>  Default_named_parameters;

#ifndef CGAL_NO_DEPRECATED_CODE
Default_named_parameters
inline all_default()
{
  return Default_named_parameters();
}
#endif

Default_named_parameters
inline default_values()
{
  return Default_named_parameters();
}

template <typename T, typename Tag, typename Base>
Named_function_parameters<T,Tag,Base>
inline no_parameters(Named_function_parameters<T,Tag,Base>)
{
  typedef Named_function_parameters<T,Tag,Base> Params;
  return Params();
}

// define free functions for named parameters
#define CGAL_add_named_parameter(X, Y, Z)        \
  template <typename K>                        \
  Named_function_parameters<K, internal_np::X>                  \
  Z(const K& p)                                \
  {                                            \
    typedef Named_function_parameters<K, internal_np::X> Params;\
    return Params(p);                          \
  }

#include <CGAL/STL_Extension/internal/parameters_interface.h>
#undef CGAL_add_named_parameter

#ifndef CGAL_NO_DEPRECATED_CODE
template <class Tag>
struct Boost_parameter_compatibility_wrapper
{
  template <typename K>
  Named_function_parameters<K, Tag>
  operator()(const K& p) const
  {
    typedef Named_function_parameters<K, Tag> Params;
    return Params(p);
  }

  template <typename K>
  Named_function_parameters<K, Tag>
  operator=(const K& p) const
  {
    typedef Named_function_parameters<K, Tag> Params;
    return Params(p);
  }
};

// TODO: need to make sure this works when using several compilation units
const Boost_parameter_compatibility_wrapper<internal_np::number_of_iterations_t> max_iteration_number_new;
const Boost_parameter_compatibility_wrapper<internal_np::convergence_ratio_t> convergence_new;
const Boost_parameter_compatibility_wrapper<internal_np::vertex_freeze_bound_t> freeze_bound_new;
const Boost_parameter_compatibility_wrapper<internal_np::maximum_running_time_t> time_limit_new;
const Boost_parameter_compatibility_wrapper<internal_np::i_seed_begin_iterator_t> seeds_begin_new;
const Boost_parameter_compatibility_wrapper<internal_np::i_seed_end_iterator_t> seeds_end_new;
const Boost_parameter_compatibility_wrapper<internal_np::seeds_are_in_domain_t> mark_new;
const Boost_parameter_compatibility_wrapper<internal_np::freeze_t> do_freeze_new;
const Boost_parameter_compatibility_wrapper<internal_np::sliver_criteria_t> sliver_criterion_new;
const Boost_parameter_compatibility_wrapper<internal_np::perturb_vector_t> perturbation_vector_new;
//Compatibility wrappers for exude_mesh_3.h
const Boost_parameter_compatibility_wrapper<internal_np::lower_sliver_bound_t> sliver_bound_new;
const Boost_parameter_compatibility_wrapper<internal_np::mesh_topology_number_t> mesh_topology_new;
const Boost_parameter_compatibility_wrapper<internal_np::dump_after_init_prefix_param_t> dump_after_init_prefix_new;
const Boost_parameter_compatibility_wrapper<internal_np::dump_after_refine_surface_prefix_param_t> dump_after_refine_surface_prefix_new;
const Boost_parameter_compatibility_wrapper<internal_np::dump_after_refine_prefix_param_t> dump_after_refine_prefix_new;
const Boost_parameter_compatibility_wrapper<internal_np::dump_after_glob_opt_prefix_param_t> dump_after_glob_opt_prefix_new;
const Boost_parameter_compatibility_wrapper<internal_np::dump_after_perturb_prefix_param_t> dump_after_perturb_prefix_new;
const Boost_parameter_compatibility_wrapper<internal_np::dump_after_exude_prefix_param_t> dump_after_exude_prefix_new;
const Boost_parameter_compatibility_wrapper<internal_np::number_of_initial_points_param_t> number_of_initial_points_new;
const Boost_parameter_compatibility_wrapper<internal_np::maximal_number_of_vertices_param_t> maximal_number_of_vertices_new;
const Boost_parameter_compatibility_wrapper<internal_np::nonlinear_growth_of_balls_param_t> nonlinear_growth_of_balls_new;
const Boost_parameter_compatibility_wrapper<internal_np::pointer_to_error_code_param_t> pointer_to_error_code_new;
const Boost_parameter_compatibility_wrapper<internal_np::pointer_to_stop_atomic_boolean_param_t> pointer_to_stop_atomic_boolean_new;
const Boost_parameter_compatibility_wrapper<internal_np::exude_options_param_t> exude_param_new;
const Boost_parameter_compatibility_wrapper<internal_np::perturb_options_param_t> perturb_param_new;
const Boost_parameter_compatibility_wrapper<internal_np::odt_options_param_t> odt_param_new;
const Boost_parameter_compatibility_wrapper<internal_np::lloyd_options_param_t> lloyd_param_new;
const Boost_parameter_compatibility_wrapper<internal_np::reset_options_param_t> reset_param_new;
const Boost_parameter_compatibility_wrapper<internal_np::mesh_param_t> mesh_options_param_new;
const Boost_parameter_compatibility_wrapper<internal_np::manifold_param_t> manifold_options_param_new;
const Boost_parameter_compatibility_wrapper<internal_np::features_option_param_t> features_param_new;

const Boost_parameter_compatibility_wrapper<internal_np::image_3_param_t> image_new;
const Boost_parameter_compatibility_wrapper<internal_np::iso_value_param_t> iso_value_new;
const Boost_parameter_compatibility_wrapper<internal_np::image_subdomain_index_t> image_values_to_subdomain_indices_new;
const Boost_parameter_compatibility_wrapper<internal_np::voxel_value_t> value_outside_new;
const Boost_parameter_compatibility_wrapper<internal_np::error_bound_t> relative_error_bound_new;
const Boost_parameter_compatibility_wrapper<internal_np::rng_t> p_rng_new;
const Boost_parameter_compatibility_wrapper<internal_np::null_subdomain_index_param_t> null_subdomain_index_new;
const Boost_parameter_compatibility_wrapper<internal_np::surface_patch_index_t> construct_surface_patch_index_new;
const Boost_parameter_compatibility_wrapper<internal_np::weights_param_t> weights_new;


#endif

// function to extract a parameter
template <typename T, typename Tag, typename Base, typename Query_tag>
typename internal_np::Get_param<internal_np::Named_params_impl<T, Tag, Base>, Query_tag>::type
get_parameter(const Named_function_parameters<T, Tag, Base>& np, Query_tag tag)
{
  return internal_np::get_parameter_impl(static_cast<const internal_np::Named_params_impl<T, Tag, Base>&>(np), tag);
}

template <typename T, typename Tag, typename Base, typename Query_tag>
typename internal_np::Get_param<internal_np::Named_params_impl<T, Tag, Base>, Query_tag>::reference
get_parameter_reference(const Named_function_parameters<T, Tag, Base>& np, Query_tag tag)
{
  return internal_np::get_parameter_reference_impl(
    static_cast<const internal_np::Named_params_impl<T, Tag, Base>&>(np),
    tag);
}

// Two parameters, non-trivial default value
template <typename D>
D& choose_parameter(const internal_np::Param_not_found&, D& d)
{
  return d;
}

template <typename D>
const D& choose_parameter(const internal_np::Param_not_found&, const D& d)
{
  return d;
}

template <typename D>
D choose_parameter(const internal_np::Param_not_found&, D&& d)
{
  return std::forward<D>(d);
}

template <typename T, typename D>
T& choose_parameter(T& t, D&)
{
  return t;
}

template <typename T, typename D>
const T& choose_parameter(const T& t, const D&)
{
  return t;
}

// single parameter so that we can avoid a default construction
template <typename D>
D choose_parameter(const internal_np::Param_not_found&)
{
  return D();
}

template <typename D, typename T>
const T& choose_parameter(const T& t)
{
  return t;
}

template <class NamedParameters, class Parameter>
struct is_default_parameter
{
  typedef typename internal_np::Lookup_named_param_def<Parameter,
                                                       NamedParameters,
                                                       internal_np::Param_not_found>::type NP_type;

  static const bool value = boost::is_same<NP_type, internal_np::Param_not_found>::value;

  typedef CGAL::Boolean_tag<value> type;
};

} // end of parameters namespace

namespace internal_np {

template <typename Tag, typename K, typename ... NPS>
auto
combine_named_parameters(const Named_function_parameters<K, Tag>& np, const NPS& ... nps)
{
  return np.combine(nps ...);
}

} // end of internal_np namespace

#ifndef CGAL_NO_DEPRECATED_CODE
namespace Polygon_mesh_processing {

namespace parameters = CGAL::parameters;

}
#endif

} //namespace CGAL

#ifndef CGAL_NO_STATIC_ASSERTION_TESTS
// code added to avoid silent runtime issues in non-updated code
namespace boost
{
  template <typename T, typename Tag, typename Base, typename Tag2, bool B = false>
  void get_param(CGAL::Named_function_parameters<T,Tag,Base>, Tag2)
  {
    CGAL_static_assertion(B && "You must use CGAL::parameters::get_parameter instead of boost::get_param");
  }
}
#endif

#endif // CGAL_BOOST_FUNCTION_PARAMS_HPP
