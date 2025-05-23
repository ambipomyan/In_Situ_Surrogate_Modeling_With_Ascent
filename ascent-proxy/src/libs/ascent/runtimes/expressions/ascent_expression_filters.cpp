//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
// Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
// other details. No copyright assignment is required to contribute to Ascent.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//-----------------------------------------------------------------------------
///
/// file: ascent_expression_filters.cpp
///
//-----------------------------------------------------------------------------

#include "ascent_expression_filters.hpp"

//-----------------------------------------------------------------------------
// thirdparty includes
//-----------------------------------------------------------------------------

// conduit includes
#include <conduit.hpp>

//-----------------------------------------------------------------------------
// ascent includes
//-----------------------------------------------------------------------------
#include "ascent_blueprint_architect.hpp"
#include "ascent_data_binning.hpp"
#include "ascent_blueprint_device_reductions.hpp"
#include "ascent_execution_manager.hpp"
#include <ascent_config.h>
#include <ascent_logging.hpp>
#include <ascent_data_object.hpp>
#include <utils/ascent_mpi_utils.hpp>
#include <flow_graph.hpp>
#include <flow_timer.hpp>
#include <flow_workspace.hpp>

#include <limits>
#include <math.h>
#include <cmath>
#include <typeinfo>

#if defined(ASCENT_DRAY_ENABLED)
#include <dray/queries/lineout.hpp>
#endif

#include <conduit_relay.hpp>

#ifdef ASCENT_MPI_ENABLED
#include <mpi.h>
#include "conduit_relay_mpi_io_blueprint.hpp"
#endif

using namespace conduit;
using namespace std;

using namespace flow;

//-----------------------------------------------------------------------------
// -- begin ascent:: --
//-----------------------------------------------------------------------------
namespace ascent
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime --
//-----------------------------------------------------------------------------
namespace runtime
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime::expressions --
//-----------------------------------------------------------------------------
namespace expressions
{

namespace detail
{
// We want to allow some objects to have basic
// attributes like vectors, but since its a base
// type, its overly burdensome to always set these
// as the return types in every filter. Thus, do this.
void fill_attrs(conduit::Node &obj)
{
  const std::string type = obj["type"].as_string();
  if(type == "vector")
  {
    double *vals = obj["value"].value();
    obj["attrs/x/value"] = vals[0];
    obj["attrs/x/type"] = "double";
    obj["attrs/y/value"] = vals[1];
    obj["attrs/y/type"] = "double";
    obj["attrs/z/value"] = vals[2];
    obj["attrs/z/type"] = "double";
  }
}

bool
is_math(const std::string &op)
{
  return op == "*" || op == "+" || op == "/" || op == "-" || op == "%";
}

bool
is_logic(const std::string &op)
{
  return op == "or" || op == "and" || op == "not";
}

bool
is_scalar(const std::string &type)
{
  return type == "int" || type == "double" || type == "scalar";
}

void
vector_op(const double lhs[3],
          const double rhs[3],
          const std::string &op,
          double res[3])
{

  if(op == "+")
  {
    res[0] = lhs[0] + rhs[0];
    res[1] = lhs[1] + rhs[1];
    res[2] = lhs[2] + rhs[2];
  }
  else if(op == "-")
  {
    res[0] = lhs[0] - rhs[0];
    res[1] = lhs[1] - rhs[1];
    res[2] = lhs[2] - rhs[2];
  }
  else
  {
    ASCENT_ERROR("Unsupported vector op " << op);
  }
}

template <typename T>
T
math_op(const T lhs, const T rhs, const std::string &op)
{
  ASCENT_ERROR("unknown type: " << typeid(T).name());
}

template <>
double
math_op(const double lhs, const double rhs, const std::string &op)
{
  double res;
  if(op == "+")
  {
    res = lhs + rhs;
  }
  else if(op == "-")
  {
    res = lhs - rhs;
  }
  else if(op == "*")
  {
    res = lhs * rhs;
  }
  else if(op == "/")
  {
    res = lhs / rhs;
  }
  else
  {
    ASCENT_ERROR("unknown math op " << op << " for type double");
  }
  return res;
}

template <>
int
math_op(const int lhs, const int rhs, const std::string &op)
{
  int res;
  if(op == "+")
  {
    res = lhs + rhs;
  }
  else if(op == "-")
  {
    res = lhs - rhs;
  }
  else if(op == "*")
  {
    res = lhs * rhs;
  }
  else if(op == "/")
  {
    res = lhs / rhs;
  }
  else if(op == "%")
  {
    res = lhs % rhs;
  }
  else
  {
    ASCENT_ERROR("unknown math op " << op << " for type int");
  }
  return res;
}

bool
comp_op(const double lhs, const double rhs, const std::string &op)
{
  int res;
  if(op == "<")
  {
    res = lhs < rhs;
  }
  else if(op == "<=")
  {
    res = lhs <= rhs;
  }
  else if(op == ">")
  {
    res = lhs > rhs;
  }
  else if(op == ">=")
  {
    res = lhs >= rhs;
  }
  else if(op == "==")
  {
    res = lhs == rhs;
  }
  else if(op == "!=")
  {
    res = lhs != rhs;
  }
  else
  {
    ASCENT_ERROR("unknown comparison op " << op);
  }

  return res;
}

bool
logic_op(const bool lhs, const bool rhs, const std::string &op)
{
  bool res;
  if(op == "or")
  {
    res = lhs || rhs;
  }
  else if(op == "and")
  {
    res = lhs && rhs;
  }
  else if(op == "not")
  {
    // a dummy lhs is being passed
    res = !rhs;
  }
  else
  {
    ASCENT_ERROR("unknown boolean op " << op);
  }
  return res;
}


void get_first_and_last_index(const string &operator_name,
                              const conduit::Node &history,
                              const int &entries,
                              const conduit::Node *n_first_index,
                              const conduit::Node *n_last_index,
                              bool absolute,
                              bool relative,
                              bool simulation_time,
                              bool simulation_cycle,
                              int &first_index,
                              int &last_index)
{

  if(absolute || relative)
  {
    first_index = (*n_first_index)["value"].to_int32();
    last_index = (*n_last_index)["value"].to_int32();

    if(first_index < 0 || last_index < 0)
    {
      ASCENT_ERROR(operator_name + ": the first index and last index must both be non-negative integers.");
    }

    if(first_index > last_index)
    {
      ASCENT_ERROR(operator_name + ": the first index must not be greater than the last index.");
    }

    if(relative) {
      int relative_first_index = first_index;
      int relative_last_index = last_index;
      //when retrieving from m to n cycles ago, where m < n, n will have a lower history index than m
      first_index = entries - relative_last_index - 1;
      last_index = entries - relative_first_index - 1;
      //clamp it to the first cycle
      if(first_index < 0) {
        first_index = 0;
      }
    }
    else
    {
      //clamp it to the last cycle
      if(last_index >= entries)
      {
        last_index = entries - 1;
      }
    }
  }
  else if(simulation_time)
  {
    double first_time = (*n_first_index)["value"].to_float64();
    double last_time = (*n_last_index)["value"].to_float64();

    // we might want window relative to the current time, and
    // if that window is larger than what we currently have,
    // clamp the values to 0
    // Another possible solution to this is to specify relative times
    first_time = std::max(0.0, first_time);
    last_time = std::max(0.0, last_time);

    if(first_time < 0 || last_time < 0)
    {
      ASCENT_ERROR(operator_name + ": the first_absolute_time and last_absolute_time "<<
                   " must both be non-negative.");
    }

    if(first_time > last_time)
    {
      ASCENT_ERROR(operator_name +
                   ": the first_absolute_time must not be "
                   <<"greater than the last_absolute_time.");
    }

    string time_path = "time";

    double time;
    last_index = 0;
    for(int index = 0; index < entries; index++)
    {
      if(history.child(index).has_path(time_path))
      {
        time = history.child(index)[time_path].to_float64();
      }
      else
      {
        ASCENT_ERROR(operator_name << ": internal error. missing " << time_path
                     << " value for time point in retrieval window (for the"
                     <<" calculation at absolute index: " + to_string(index) + ")." );
      }

      // I am not totally sure about this logic. Part of the problem is that we
      // haven't fully specified what we want this behavior to be.
      if(first_index == -1 && time >= first_time)
      {
        first_index = index;
      }
      if(time <= last_time)
      {
        last_index = index;
      }
      if(time > last_time)
      {
        break;
      }
    }
    //clamp it to the last index to at least the first index
    if(last_index < first_index)
    {
      last_index = first_index;
    }
  }
  else if(simulation_cycle)
  {
    long long first_cycle = (*n_first_index)["value"].to_int64();
    long long last_cycle = (*n_last_index)["value"].to_int64();

    if(first_cycle < 0 || last_cycle < 0)
    {
      ASCENT_ERROR(operator_name + ": the first_absolute_cycle and last_absolute_cycle must both be non-negative.");
    }

    if(first_cycle > last_cycle)
    {
      ASCENT_ERROR(operator_name + ": the first_absolute_cycle must not be greater than the last_absolute_cycle.");
    }

    vector<string> child_names = history.child_names();
    if(child_names.size() != entries)
    {
      ASCENT_ERROR(operator_name + ": internal error. number of history entries: "
                   <<to_string(entries) << ", but number of history child names: "
                   << to_string(child_names.size()));
    }

    unsigned long long cycle;
    for(int index = 0; index < entries; index++)
    {
      cycle = stoull(child_names[index]);
      if(first_index == -1 && cycle >= first_cycle)
      {
        first_index = index;
      }
      else if(cycle > last_cycle)
      {
        last_index = index - 1;
        break;
      }
    }
    //clamp it to the last index
    if(last_index == -1)
    {
      last_index = entries - 1;
    }
  }
}

void set_values_from_history(const string &operator_name,
                             const conduit::Node &history,
                             int first_index,
                             int return_size,
                             bool return_history_index,
                             bool return_simulation_time,
                             bool return_simulation_cycle,
                             conduit::Node *output)
{

  bool gradient = (return_history_index || return_simulation_time || return_simulation_cycle);

  string value_path = "";
  vector<string> value_paths = {"value", "attrs/value/value"};
  for(const string &path : value_paths)
  {
    if(history.child(first_index).has_path(path))
    {
      value_path = path;
      break;
    }
  }


  if(value_path.size() == 0)
  {
    ASCENT_ERROR(operator_name + " internal error. first index does not have one of the expected value paths");
  }

  conduit::DataType dtype = history.child(first_index)[value_path].dtype();

  if(dtype.is_float32())
  {
    float *array = new float[return_size];
    for(int i = 0; i < return_size; ++i)
    {
      array[i] = history.child(first_index+i)[value_path].to_float32();
    }
    (*output)["value"].set(array, return_size);
    delete[] array;
  }
  else if(dtype.is_float64())
  {
    double *array = new double[return_size];
    for(int i = 0; i < return_size; ++i)
    {
      array[i] = history.child(first_index+i)[value_path].to_float64();
    }
    (*output)["value"].set(array, return_size);
    delete[] array;
  }
  else if(dtype.is_int32())
  {
    int *array = new int[return_size];
    for(int i = 0; i < return_size; ++i)
    {
      array[i] = history.child(first_index+i)[value_path].to_int32();
    }
    (*output)["value"].set(array, return_size);
    delete[] array;
  }
  else if(dtype.is_int64())
  {
    long long *array = new long long[return_size];
    for(int i = 0; i < return_size; ++i)
    {
      array[i] = history.child(first_index+i)[value_path].to_int64();
    }
    (*output)["value"].set(array, return_size);
    delete[] array;
  }
  else
  {
    ASCENT_ERROR(operator_name + ": unsupported array type "<< dtype.to_string());
  }
  (*output)["type"] = "array";

  if(gradient)
  {
    if(return_history_index)
    {
      long long *index_array = new long long[return_size];
      for(int i = 0; i < return_size-1; ++i)
      {
          index_array[i] = 1;
      }
      (*output)["time"].set(index_array, return_size-1);
      delete[] index_array;
    }
    else if(return_simulation_time)
    {
      double *simulation_time_array = new double[return_size];
      for(int i = 0; i < return_size-1; ++i)
      {
        simulation_time_array[i]
          = history.child(first_index + i + 1)["time"].to_float64() - history.child(first_index + i)["time"].to_float64();

      }
      (*output)["time"].set(simulation_time_array, return_size-1);
      delete[] simulation_time_array;
    }
    else if(return_simulation_cycle)
    {
      vector<string> child_names = history.child_names();
      long long *cycle_array = new long long[return_size];
      for(int i = 0; i < return_size-1; ++i)
      {
          cycle_array[i] = stoll(child_names[first_index + i + 1]) - stoll(child_names[first_index + i]);
      }
      (*output)["time"].set(cycle_array, return_size-1);
      delete[] cycle_array;
    }
  }
}


conduit::Node *
range_values_helper(const conduit::Node &history,
                    const conduit::Node *n_first_absolute_index,
                    const conduit::Node *n_last_absolute_index,
                    const conduit::Node *n_first_relative_index,
                    const conduit::Node *n_last_relative_index,
                    const conduit::Node *n_first_absolute_time,
                    const conduit::Node *n_last_absolute_time,
                    const conduit::Node *n_first_absolute_cycle,
                    const conduit::Node *n_last_absolute_cycle,
                    const string &operator_name,
                    const std::string time_units = "")  // 'cycle' 'index' 'time'
{
  conduit::Node *output = new conduit::Node();

  bool absolute =
    (!n_first_absolute_index->dtype().is_empty() || !n_last_absolute_index->dtype().is_empty());
  bool relative =
    (!n_first_relative_index->dtype().is_empty() || !n_last_relative_index->dtype().is_empty());
  bool simulation_time =
    (!n_first_absolute_time->dtype().is_empty() || !n_last_absolute_time->dtype().is_empty());
  bool simulation_cycle =
    (!n_first_absolute_cycle->dtype().is_empty() || !n_last_absolute_cycle->dtype().is_empty());

  int count = absolute + relative + simulation_time + simulation_cycle;

  if(count == 0)
  {
    ASCENT_ERROR(
        operator_name << ": Must specify a selection range, providing either "
                      <<"1). first_absolute_index and last_absolute_index, "
                      <<"2). first_relative_index and last_relative_index, :"
                      <<"3). first_absolute_time and last_absolute_time, or "
                      <<"4). first_absolute_cycle and last_absolute_cycle.");
  }

  if(count > 1)
  {
    ASCENT_ERROR(
        operator_name <<": Must specify exactly one selection range, providing either "
                      <<"1). first_absolute_index and last_absolute_index, "
                      <<"2). first_relative_index and last_relative_index, "
                      <<"3). first_absolute_time and last_absolute_time, or "
                      <<"4). first_absolute_cycle and last_absolute_cycle.");
  }

  const conduit::Node *n_first_index, *n_last_index;
  int first_index = -1, last_index = -1;
  if(absolute)
  {
    n_first_index = n_first_absolute_index;
    n_last_index = n_last_absolute_index;
  }
  else if(relative)
  {
    n_first_index = n_first_relative_index;
    n_last_index = n_last_relative_index;
  }
  else if(simulation_cycle)
  {
    n_first_index = n_first_absolute_cycle;
    n_last_index = n_last_absolute_cycle;
  }
  else if(simulation_time)
  {
    n_first_index = n_first_absolute_time;
    n_last_index = n_last_absolute_time;
  }

  const int entries = history.number_of_children();
  if(entries <= 0)
  {
    ASCENT_ERROR(
        operator_name + ": no entries collected for expression.");
  }

  if(!n_first_index->has_path("value"))
  {
    ASCENT_ERROR(
        operator_name + ": internal error. first_index does not have child value");
  }
  if(!n_last_index->has_path("value"))
  {
    ASCENT_ERROR(
        operator_name + ": internal error. last_index does not have child value");
  }

  get_first_and_last_index(operator_name,
                          history,
                          entries,
                          n_first_index,
                          n_last_index,
                          absolute,
                          relative,
                          simulation_time,
                          simulation_cycle,
                          first_index,
                          last_index);

  //the entire range falls outside what has been recorded so far
  if(first_index < 0 && last_index < 0)
  {
    return output;
  }

  const int return_size = last_index - first_index + 1;

  bool return_history_index = time_units == "index";
  bool return_simulation_time = time_units == "time";
  bool return_simulation_cycle = time_units == "cycle";

  set_values_from_history(operator_name,
                          history,
                          first_index,
                          return_size,
                          return_history_index,
                          return_simulation_time,
                          return_simulation_cycle,
                          output);

  return output;
}


} // namespace detail

void resolve_symbol_result(flow::Graph &graph,
                           const conduit::Node *output,
                           const std::string filter_name)
{
  conduit::Node *symbol_table =
    graph.workspace().registry().fetch<conduit::Node>("symbol_table");
  const int num_symbols = symbol_table->number_of_children();
  for(int i = 0; i < num_symbols; ++i)
  {
    conduit::Node &symbol = symbol_table->child(i);
    if(symbol["filter_name"].as_string() == filter_name)
    {
      symbol["value"] = output->fetch("value");
      break;
    }
  }
}
void binning_interface(const std::string &reduction_var,
                       const std::string &reduction_op,
                       const conduit::Node &n_empty_bin_val,
                       const conduit::Node &n_component,
                       const conduit::Node &n_axis_list,
                       conduit::Node &dataset,
                       conduit::Node &n_binning,
                       conduit::Node &n_output_axes)
{
  std::string component = "";
  if(!n_component.dtype().is_empty())
  {
    component = n_component["value"].as_string();
  }

  if(!n_axis_list.has_path("type"))
  {
    ASCENT_ERROR("Binning: axis list missing object type.");
  }
  std::string obj_type = n_axis_list["type"].as_string();
  if(obj_type != "list")
  {
    ASCENT_ERROR("Binning: axis list is not type 'list'."
                  <<" type is '"<<obj_type<<"'");
  }
  // verify n_axes_list and put the values in n_output_axes
  int num_axes = n_axis_list["value"].number_of_children();
  for(int i = 0; i < num_axes; ++i)
  {
    const conduit::Node &axis = n_axis_list["value"].child(i);
    if(axis["type"].as_string() != "axis")
    {
      ASCENT_ERROR("Binning: bin_axes must be a list of axis");
    }
    n_output_axes.update(axis["value"]);
  }

  // verify reduction_var
  if(reduction_var.empty())
  {
    if(reduction_op != "count" && reduction_op != "pdf")
    {
      ASCENT_ERROR("Binning: reduction_var can only be left empty if "
                   "reduction_op is 'count' or 'pdf'.");
    }
  }
  else if(!is_xyz(reduction_var))
  {
    if(!has_field(dataset, reduction_var))
    {
      std::string known;
      if(dataset.number_of_children() > 0 )
      {
        std::vector<std::string> names = dataset.child(0)["fields"].child_names();
        std::stringstream ss;
        ss << "[";
        for(size_t i = 0; i < names.size(); ++i)
        {
          ss << " '" << names[i]<<"'";
        }
        ss << "]";
        known = ss.str();
      }
      ASCENT_ERROR("Binning: reduction variable '"
                   << reduction_var
                   << "' must be a scalar field in the dataset or x/y/z or empty."
                   << " known = " << known);
    }

    bool scalar = is_scalar_field(dataset, reduction_var);
    if(!scalar && component == "")
    {
      ASCENT_ERROR("Binning: reduction variable '"
                   << reduction_var <<"'"
                   << " has multiple components and no 'component' is"
                   << " specified."
                   << " known components = "
                   << possible_components(dataset, reduction_var));
    }
    if(scalar && component != "")
    {
      ASCENT_ERROR("Binning: reduction variable '"
                   << reduction_var <<"'"
                   << " is a scalar(i.e., has not components "
                   << " but 'component' " << " '"<<component<<"' was"
                   << " specified. Remove the 'component' argument"
                   << " or choose a vector variable.");
    }
    if(!has_component(dataset, reduction_var, component))
    {
      ASCENT_ERROR("Binning: reduction variable '"
                   << reduction_var << "'"
                   << " does not have component '"<<component<<"'."
                   << " known components = "
                   << possible_components(dataset, reduction_var));

    }
  }

  // verify reduction_op
  if(reduction_op != "sum" && reduction_op != "min" && reduction_op != "max" &&
     reduction_op != "avg" && reduction_op != "pdf" && reduction_op != "std" &&
     reduction_op != "var" && reduction_op != "rms" && reduction_op != "count")
  {
    ASCENT_ERROR(
        "Unknown reduction_op: '"
        << reduction_op
        << "'. Known reduction operators are: sum, min, max, avg, count,"
           " pdf, std, var, rms");
  }

  double empty_bin_val = 0;
  if(!n_empty_bin_val.dtype().is_empty())
  {
    empty_bin_val = n_empty_bin_val["value"].to_float64();
  }

  n_binning = binning(dataset,
                      n_output_axes,
                      reduction_var,
                      reduction_op,
                      empty_bin_val,
                      component);

  // // TODO THIS IS THE RAJA VERSION
  // std::map<int, Array<int>> bindexes;
  // n_binning = data_binning(dataset,
  //                          n_output_axes,
  //                          reduction_var,
  //                          reduction_op,
  //                          empty_bin_val,
  //                          component,
  //                          bindexes);


// TODO: Internal work testing the raja version samples
// // void data_binning_samples(conduit::Node &dataset,
// //                           conduit::Node &bin_axes,
// //                           const std::string &reduction_var,
// //                           const std::string &reduction_op,
// //                           const double empty_bin_val,
// //                           const std::string &component,
// //                           std::map<int,Array<double> > &points,
// //                           std::map<int,Array<double> > &values);
//
//   std::map<int, Array<double>> sample_points;
//   std::map<int, Array<double>> sample_values;
//   data_binning_samples(dataset,
//                       n_output_axes,
//                       reduction_var,
//                       reduction_op,
//                       empty_bin_val,
//                       component,
//                       sample_points,
//                       sample_values);
//
//   Node bp_samples_mesh;
//
//   std::cout << " GAH:  points stuff" << std::endl;
//   for( const auto &pts_items : sample_points )
//   {
//      Node &dom = bp_samples_mesh.append();
//      index_t npts = pts_items.second.size() / 3;
//      dom["state/domain_id"] = pts_items.first;
//
//      dom["coordsets/coords/type"] = "explicit";
//      dom["coordsets/coords/values/x"].set(DataType::float64(npts));
//      dom["coordsets/coords/values/y"].set(DataType::float64(npts));
//      dom["coordsets/coords/values/z"].set(DataType::float64(npts));
//      dom["topologies/points/coordset"] = "coords";
//      dom["topologies/points/type"] = "points";
//
//      float64_array x_vals = dom["coordsets/coords/values/x"].value();
//      float64_array y_vals = dom["coordsets/coords/values/y"].value();
//      float64_array z_vals = dom["coordsets/coords/values/z"].value();
//      // int idx = 0;
//      // for(index_t i=0;i<npts;i++)
//      // {
//      //   x_vals[i] = pts_items.second.get_value(idx);
//      //   y_vals[i] = pts_items.second.get_value(idx+1);
//      //   z_vals[i] = pts_items.second.get_value(idx+2);
//      //   idx+=3;
//      // }
//
//      int idx = 0;
//      for(index_t i=0;i<npts;i++)
//      {
//        x_vals[i] = pts_items.second.get_value(idx);
//        idx+=1;
//      }
//      for(index_t i=0;i<npts;i++)
//      {
//        y_vals[i] = pts_items.second.get_value(idx);
//        idx+=1;
//      }
//      for(index_t i=0;i<npts;i++)
//      {
//        z_vals[i] = pts_items.second.get_value(idx);
//        idx+=1;
//      }
//
//   }
//
//   Node opts;
//   #ifdef ASCENT_MPI_ENABLED
//     MPI_Comm mpi_comm = MPI_Comm_f2c(flow::Workspace::default_mpi_comm());
//     conduit::relay::mpi::io::blueprint::save_mesh(bp_samples_mesh,
//                                                 "here_we_go",
//                                                 "hdf5",
//                                                 opts,
//                                                 mpi_comm);
//   #else
//     conduit::relay::io::blueprint::save_mesh(bp_samples_mesh,
//                                                 "here_we_go",
//                                                 "hdf5",
//                                                 opts);
//   #endif
//
//
//   std::cout << " GAH:  vals stuff" << std::endl;
//   for( const auto &vals_items : sample_values )
//   {
//       std::cout << vals_items.first <<  " " << vals_items.second.size() << "\n";
//
//   }


}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Base Language Components
// boolean, integer, double, string, nan, null, identifier, 
// if (conditional), binary operations (math, logical, comparison etc)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprBoolean
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprBoolean::ExprBoolean()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprBoolean::~ExprBoolean()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprBoolean::declare_interface(Node &i)
{
  i["type_name"] = "expr_bool";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprBoolean::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  if(!params.has_path("value"))
  {
    info["errors"].append() = "Missing required numeric parameter 'value'";
    res = false;
  }
  return res;
}

//-----------------------------------------------------------------------------
void
ExprBoolean::execute()
{
  conduit::Node *output = new conduit::Node();
  (*output)["value"] = params()["value"].to_uint8();
  (*output)["type"] = "bool";
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprInteger
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprInteger::ExprInteger() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprInteger::~ExprInteger()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprInteger::declare_interface(Node &i)
{
  i["type_name"] = "expr_integer";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprInteger::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  if(!params.has_path("value"))
  {
    info["errors"].append() = "Missing required numeric parameter 'value'";
    res = false;
  }
  return res;
}

//-----------------------------------------------------------------------------
void
ExprInteger::execute()
{
  conduit::Node *output = new conduit::Node();
  (*output)["value"] = params()["value"].to_int32();
  (*output)["type"] = "int";
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprDouble
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprDouble::ExprDouble()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprDouble::~ExprDouble()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprDouble::declare_interface(Node &i)
{
  i["type_name"] = "expr_double";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprDouble::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  if(!params.has_path("value"))
  {
    info["errors"].append() = "Missing required numeric parameter 'value'";
    res = false;
  }
  return res;
}

//-----------------------------------------------------------------------------
void
ExprDouble::execute()
{
  conduit::Node *output = new conduit::Node();
  (*output)["value"] = params()["value"].to_float64();
  (*output)["type"] = "double";
  set_output<conduit::Node>(output);
}


//*****************************************************************************
// ExprString
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprString::ExprString()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprString::~ExprString()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprString::declare_interface(Node &i)
{
  i["type_name"] = "expr_string";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprString::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  if(!params.has_path("value"))
  {
    info["errors"].append() = "Missing required string parameter 'value'";
    res = false;
  }
  return res;
}

//-----------------------------------------------------------------------------
void
ExprString::execute()
{
  conduit::Node *output = new conduit::Node();

  (*output)["value"] = params()["value"].as_string();
  (*output)["type"] = "string";
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprNan
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprNan::ExprNan() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprNan::~ExprNan()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprNan::declare_interface(Node &i)
{
  i["type_name"] = "expr_nan";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprNan::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprNan::execute()
{
  conduit::Node *output = new conduit::Node();

  (*output)["type"] = "double";
  (*output)["value"] = nan("");;
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprNull
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprNull::ExprNull()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprNull::~ExprNull()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprNull::declare_interface(Node &i)
{
  i["type_name"] = "expr_null";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprNull::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprNull::execute()
{
  conduit::Node *output = new conduit::Node();
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprIdentifier
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprIdentifier::ExprIdentifier()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprIdentifier::~ExprIdentifier()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprIdentifier::declare_interface(Node &i)
{
  i["type_name"] = "expr_identifier";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprIdentifier::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  if(!params.has_path("value"))
  {
    info["errors"].append() = "Missing required string parameter 'value'";
    res = false;
  }
  return res;
}

//-----------------------------------------------------------------------------
void
ExprIdentifier::execute()
{
  conduit::Node *output = new conduit::Node();
  std::string i_name = params()["value"].as_string();

  const conduit::Node *const cache =
      graph().workspace().registry().fetch<Node>("cache");
  if(!cache->has_path(i_name))
  {
    ASCENT_ERROR("Unknown expression identifier: '" << i_name << "'");
  }

  const int entries = (*cache)[i_name].number_of_children();
  if(entries < 1)
  {
    ASCENT_ERROR("Expression identifier: needs at least one entry");
  }
  // grab the last one calculated so we have type info
  (*output) = (*cache)[i_name].child(entries - 1);
  // we need to keep the name to retrieve the cache
  // if history is called.
  (*output)["name"] = i_name;

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprObjectDotAccess
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprObjectDotAccess::ExprObjectDotAccess() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprObjectDotAccess::~ExprObjectDotAccess()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprObjectDotAccess::declare_interface(Node &i)
{
  i["type_name"] = "expr_dot";
  i["port_names"].append() = "obj";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprObjectDotAccess::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  if(!params.has_path("name"))
  {
    info["errors"].append() = "DotAccess: Missing required parameter 'name'";
    res = false;
  }
  return res;
}

//-----------------------------------------------------------------------------
void
ExprObjectDotAccess::execute()
{
  conduit::Node *n_obj = input<Node>("obj");
  std::string name = params()["name"].as_string();

  conduit::Node *output = new conduit::Node();

  // fills attrs for basic types like vectors
  detail::fill_attrs(*n_obj);

  // TODO test accessing non-existent attribute
  if(!n_obj->has_path("attrs/" + name))
  {
    n_obj->print();
    std::stringstream ss;
    if(n_obj->has_path("attrs"))
    {
      std::string attr_yaml = (*n_obj)["attrs"].to_yaml();
      if(attr_yaml == "")
      {
        ss<<" No known attributes.";
      }
      else
      {
        ss<<" Known attributes: "<<attr_yaml;
      }
    }
    else
    {
      ss<<" No known attributes.";
    }

    ASCENT_ERROR("'"<<name << "' is not a valid object attribute for"
                      <<" type '"<<(*n_obj)["type"].as_string()<<"'."
                      <<ss.str());
  }

  (*output) = (*n_obj)["attrs/" + name];

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprIf
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprIf::ExprIf()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprIf::~ExprIf()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprIf::declare_interface(Node &i)
{
  i["type_name"] = "expr_if";
  i["port_names"].append() = "condition";
  i["port_names"].append() = "if";
  i["port_names"].append() = "else";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprIf::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprIf::execute()
{
  conduit::Node *n_condition = input<Node>("condition");
  conduit::Node *n_if = input<Node>("if");
  conduit::Node *n_else = input<Node>("else");

  conduit::Node *output;
  if((*n_condition)["value"].to_uint8() == 1)
  {
    output = n_if;
  }
  else
  {
    output = n_else;
  }

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}


//*****************************************************************************
// ExprBinaryOp
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprBinaryOp::ExprBinaryOp()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprBinaryOp::~ExprBinaryOp()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprBinaryOp::declare_interface(Node &i)
{
  i["type_name"] = "expr_binary_op";
  i["port_names"].append() = "lhs";
  i["port_names"].append() = "rhs";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprBinaryOp::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  if(!params.has_path("op_string"))
  {
    info["errors"].append() = "Missing required string parameter 'op_string'";
    res = false;
  }
  return res;
}

//-----------------------------------------------------------------------------
void
ExprBinaryOp::execute()
{
  const conduit::Node *n_lhs = input<Node>("lhs");
  const conduit::Node *n_rhs = input<Node>("rhs");

  const conduit::Node &lhs = (*n_lhs)["value"];
  const conduit::Node &rhs = (*n_rhs)["value"];

  std::string op_str = params()["op_string"].as_string();
  const std::string l_type = (*n_lhs)["type"].as_string();
  const std::string r_type = (*n_rhs)["type"].as_string();

  conduit::Node *output = new conduit::Node();
  std::stringstream msg;

  if(detail::is_math(op_str))
  {
    if(detail::is_scalar(l_type) && detail::is_scalar(r_type))
    {
      // promote to double if at least one is a double
      if(l_type == "double" || r_type == "double")
      {
        double d_rhs = rhs.to_float64();
        double d_lhs = lhs.to_float64();
        (*output)["value"] = detail::math_op(d_lhs, d_rhs, op_str);
        (*output)["type"] = "double";
      }
      else
      {
        int i_rhs = rhs.to_int32();
        int i_lhs = lhs.to_int32();
        (*output)["value"] = detail::math_op(i_lhs, i_rhs, op_str);
        (*output)["type"] = "int";
      }
    }
    else
    {
      if(detail::is_scalar(l_type) != detail::is_scalar(r_type))
      {
        msg << "' " << l_type << " " << op_str << " " << r_type << "'";
        ASCENT_ERROR(
            "Mixed vector and scalar quantities not implemented / supported: "
            << msg.str());
      }

      double res[3];
      detail::vector_op(lhs.value(), rhs.value(), op_str, res);

      (*output)["value"].set(res, 3);
      (*output)["type"] = "vector";
    }
  }
  else if(detail::is_logic(op_str))
  {

    bool b_lhs = lhs.to_uint8();
    bool b_rhs = rhs.to_uint8();
    (*output)["value"] = detail::logic_op(b_lhs, b_rhs, op_str);
    (*output)["type"] = "bool";
  }
  else // comparison
  {
    double d_rhs = rhs.to_float64();
    double d_lhs = lhs.to_float64();

    (*output)["value"] = detail::comp_op(d_lhs, d_rhs, op_str);
    (*output)["type"] = "bool";
  }

  // std::cout<<" operation "<<op_str<<"\n";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//---------------------------------------------------------------------------//
// Scalar Operations
//  min, max, abs, exp, log, pow
//---------------------------------------------------------------------------//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprScalarMin
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprScalarMin::ExprScalarMin() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprScalarMin::~ExprScalarMin()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprScalarMin::declare_interface(Node &i)
{
  i["type_name"] = "expr_scalar_min";
  i["port_names"].append() = "arg1";
  i["port_names"].append() = "arg2";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprScalarMin::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprScalarMin::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");
  const conduit::Node *arg2 = input<Node>("arg2");

  conduit::Node *output = new conduit::Node();

  if((*arg1)["type"].as_string() == "double" ||
     (*arg2)["type"].as_string() == "double")
  {
    double d_rhs = (*arg1)["value"].to_float64();
    double d_lhs = (*arg2)["value"].to_float64();
    (*output)["value"] = std::min(d_lhs, d_rhs);
    (*output)["type"] = "double";
  }
  else
  {
    int i_rhs = (*arg1)["value"].to_int32();
    int i_lhs = (*arg2)["value"].to_int32();
    (*output)["value"] = std::min(i_lhs, i_rhs);
    (*output)["type"] = "int";
  }

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprScalarMax
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprScalarMax::ExprScalarMax() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprScalarMax::~ExprScalarMax()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprScalarMax::declare_interface(Node &i)
{
  i["type_name"] = "expr_scalar_max";
  i["port_names"].append() = "arg1";
  i["port_names"].append() = "arg2";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprScalarMax::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprScalarMax::execute()
{

  const conduit::Node *arg1 = input<Node>("arg1");
  const conduit::Node *arg2 = input<Node>("arg2");

  conduit::Node *output = new conduit::Node();

  if((*arg1)["type"].as_string() == "double" ||
     (*arg2)["type"].as_string() == "double")
  {
    double d_rhs = (*arg1)["value"].to_float64();
    double d_lhs = (*arg2)["value"].to_float64();
    (*output)["value"] = std::max(d_lhs, d_rhs);
    (*output)["type"] = "double";
  }
  else
  {
    int i_rhs = (*arg1)["value"].to_int32();
    int i_lhs = (*arg2)["value"].to_int32();
    (*output)["value"] = std::max(i_lhs, i_rhs);
    (*output)["type"] = "int";
  }

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprScalarAbs
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprScalarAbs::ExprScalarAbs()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprScalarAbs::~ExprScalarAbs()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprScalarAbs::declare_interface(Node &i)
{
  i["type_name"] = "expr_scalar_abs";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprScalarAbs::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprScalarAbs::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");

  if((*arg1)["type"].as_string() == "double")
  {
    double res = 0.;
    res = abs((*arg1)["value"].to_float64());
    conduit::Node *output = new conduit::Node();
    (*output)["type"] = "double";
    (*output)["value"] = res;

    resolve_symbol_result(graph(), output, this->name());
    set_output<conduit::Node>(output);
  }
  else
  {
    int res = 0;
    res = abs((*arg1)["value"].to_int32());
    conduit::Node *output = new conduit::Node();
    (*output)["type"] = "int";
    (*output)["value"] = res;

    resolve_symbol_result(graph(), output, this->name());
    set_output<conduit::Node>(output);
  }
}


//*****************************************************************************
// ExprScalarExp
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprScalarExp::ExprScalarExp()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprScalarExp::~ExprScalarExp()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprScalarExp::declare_interface(Node &i)
{
  i["type_name"] = "expr_scalar_exp";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprScalarExp::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprScalarExp::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");

  double res = 0.;
  res = exp((*arg1)["value"].to_float64());
  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "double";
  (*output)["value"] = res;

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprScalarLog
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprScalarLog::ExprScalarLog()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprScalarLog::~ExprScalarLog()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprScalarLog::declare_interface(Node &i)
{
  i["type_name"] = "expr_scalar_log";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprScalarLog::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprScalarLog::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");

  double res = 0.;
  res = log((*arg1)["value"].to_float64());
  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "double";
  (*output)["value"] = res;

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprScalarPow
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprScalarPow::ExprScalarPow() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprScalarPow::~ExprScalarPow()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprScalarPow::declare_interface(Node &i)
{
  i["type_name"] = "expr_scalar_pow";
  i["port_names"].append() = "arg1";
  i["port_names"].append() = "arg2";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprScalarPow::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprScalarPow::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");
  const conduit::Node *arg2 = input<Node>("arg2");

  double res = 0.;
  double base = (*arg1)["value"].to_float64();
  double exponent = (*arg2)["value"].to_float64();
  res = pow(base, exponent);
  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "double";
  (*output)["value"] = res;

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Vector Operations
//   vector, vector_magnitude
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprVector
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprVector::ExprVector()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprVector::~ExprVector()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprVector::declare_interface(Node &i)
{
  i["type_name"] = "expr_vector";
  i["port_names"].append() = "arg1";
  i["port_names"].append() = "arg2";
  i["port_names"].append() = "arg3";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprVector::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprVector::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");
  const conduit::Node *arg2 = input<Node>("arg2");
  const conduit::Node *arg3 = input<Node>("arg3");

  double vec[3] = {0., 0., 0.};
  vec[0] = (*arg1)["value"].to_float64();
  vec[1] = (*arg2)["value"].to_float64();
  vec[2] = (*arg3)["value"].to_float64();

  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "vector";
  (*output)["value"].set(vec, 3);

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprVectorMagnitude
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprVectorMagnitude::ExprVectorMagnitude()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprVectorMagnitude::~ExprVectorMagnitude()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprVectorMagnitude::declare_interface(Node &i)
{
  i["type_name"] = "expr_vector_magnitude";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprVectorMagnitude::verify_params(const conduit::Node &params,
                                   conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprVectorMagnitude::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");

  double res = 0.;
  const double *vec = (*arg1)["value"].value();
  res = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "double";
  (*output)["value"] = res;

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Array Operations
//  access, replace, min, max, avg, sum
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprArrayAccess
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprArrayAccess::ExprArrayAccess()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprArrayAccess::~ExprArrayAccess()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprArrayAccess::declare_interface(Node &i)
{
  i["type_name"] = "expr_array_access";
  i["port_names"].append() = "array";
  i["port_names"].append() = "index";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprArrayAccess::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprArrayAccess::execute()
{
  const conduit::Node *n_array = input<Node>("array");
  const conduit::Node *n_index = input<Node>("index");

  conduit::Node *output = new conduit::Node();

  int index = (*n_index)["value"].as_int32();
  int length = (*n_array)["value"].dtype().number_of_elements();
  if(index > length - 1)
  {
    ASCENT_ERROR("ArrayAccess: array index out of bounds: [0," << length - 1
                                                               << "]");
  }
  const double *arr = (*n_array)["value"].value();
  (*output)["value"] = arr[index];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}


//*****************************************************************************
// ExprArrayReplace
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprArrayReplace::ExprArrayReplace()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprArrayReplace::~ExprArrayReplace()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprArrayReplace::declare_interface(Node &i)
{
  i["type_name"] = "expr_array_replace";
  i["port_names"].append() = "arg1";
  i["port_names"].append() = "find";
  i["port_names"].append() = "replace";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprArrayReplace::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprArrayReplace::execute()
{
  const conduit::Node *n_array= input<Node>("arg1");
  const conduit::Node *n_find = input<Node>("find");
  const conduit::Node *n_replace = input<Node>("replace");

  if(n_array->fetch_existing("type").as_string() != "array")
  {
    ASCENT_ERROR("replace is not an array");
  }

  if(n_find->fetch_existing("type").as_string() != "double")
  {
    ASCENT_ERROR("'find' is not a double");
  }

  if(n_replace->fetch_existing("type").as_string() != "double")
  {
    ASCENT_ERROR("'replace' is not a double");
  }

  conduit::Node *output = new conduit::Node();
  // copy the input into the ouptut
  *output = *n_array;
  conduit::Node &array = output->fetch_existing("value");

  if(!array.dtype().is_float64())
  {
    ASCENT_ERROR("Replace is only implemented for doubles");
  }

  const int size = array.dtype().number_of_elements();
  conduit::float64 *ptr =  array.as_float64_ptr();
  const double find = n_find->fetch_existing("value").to_float64();
  const double replace = n_replace->fetch_existing("value").to_float64();

  bool find_nan = find != find;

  if( !find_nan)
  {
    for(int i = 0; i < size; ++i)
    {
      if(ptr[i] == find)
      {
        ptr[i] = replace;
      }
    }
  }
  else
  {
    for(int i = 0; i < size; ++i)
    {
      if(ptr[i] != ptr[i])
      {
        ptr[i] = replace;
      }
    }
  }

  set_output<conduit::Node>(output);
}


//*****************************************************************************
// ExprArrayReductionMin
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprArrayReductionMin::ExprArrayReductionMin()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprArrayReductionMin::~ExprArrayReductionMin()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionMin::declare_interface(Node &i)
{
  i["type_name"] = "expr_array_reduction_min";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprArrayReductionMin::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionMin::execute()
{
  conduit::Node *output = new conduit::Node();
  std::string exec = ExecutionManager::preferred_cpu_policy();
  (*output)["value"] = array_min((*input<Node>("arg1"))["value"], exec)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprArrayReductionMax
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprArrayReductionMax::ExprArrayReductionMax()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprArrayReductionMax::~ExprArrayReductionMax()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionMax::declare_interface(Node &i)
{
  i["type_name"] = "expr_array_reduction_max";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprArrayReductionMax::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionMax::execute()
{
  conduit::Node *output = new conduit::Node();
  std::string exec = ExecutionManager::preferred_cpu_policy();
  (*output)["value"] = array_max((*input<Node>("arg1"))["value"], exec)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprArrayReductionAvg
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprArrayReductionAvg::ExprArrayReductionAvg()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprArrayReductionAvg::~ExprArrayReductionAvg()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionAvg::declare_interface(Node &i)
{
  i["type_name"] = "expr_array_reduction_avg";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprArrayReductionAvg::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionAvg::execute()
{
  conduit::Node *output = new conduit::Node();
  std::string exec = ExecutionManager::preferred_cpu_policy();
  conduit::Node sum = array_sum((*input<Node>("arg1"))["value"], exec);
  (*output)["value"] = sum["value"].to_float64() / sum["count"].to_float64();
  (*output)["type"] = "double";

  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprArrayReductionAvg
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprArrayReductionSum::ExprArrayReductionSum()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprArrayReductionSum::~ExprArrayReductionSum()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionSum::declare_interface(Node &i)
{
  i["type_name"] = "expr_array_reduction_sum";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprArrayReductionSum::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprArrayReductionSum::execute()
{
  conduit::Node *output = new conduit::Node();
  std::string exec = ExecutionManager::preferred_cpu_policy();
  (*output)["value"] = array_sum((*input<Node>("arg1"))["value"], exec)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// History Operations
//  history, history_range, history_gradient, history_range_gradient
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprHistory
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistory::ExprHistory()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistory::~ExprHistory()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistory::declare_interface(Node &i)
{
  i["type_name"] = "expr_history";
  i["port_names"].append() = "expr_name";
  i["port_names"].append() = "absolute_index";
  i["port_names"].append() = "relative_index";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistory::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistory::execute()
{
  conduit::Node *output = new conduit::Node();

  const std::string expr_name  = (*input<Node>("expr_name"))["name"].as_string();

  const conduit::Node *const cache =
      graph().workspace().registry().fetch<Node>("cache");

  if(!cache->has_path(expr_name))
  {
    ASCENT_ERROR("History: unknown identifier "<<  expr_name);
  }
  const conduit::Node &history = (*cache)[expr_name];

  const conduit::Node *n_absolute_index = input<Node>("absolute_index");
  const conduit::Node *n_relative_index = input<Node>("relative_index");


  if(!n_absolute_index->dtype().is_empty() &&
     !n_relative_index->dtype().is_empty())
  {
    ASCENT_ERROR(
        "History: Specify only one of relative_index or absolute_index.");
  }


  const int entries = history.number_of_children();
  if(!n_relative_index->dtype().is_empty())
  {
    int relative_index = (*n_relative_index)["value"].to_int32();
    if(relative_index >= entries)
    {
      // clamp to first if its gone too far
      relative_index = 0;
    }
    if(relative_index < 0)
    {
      ASCENT_ERROR("History: relative_index must be a non-negative integer.");
    }
    // grab the value from relative_index cycles ago
    (*output) = history.child(entries - relative_index - 1);
  }
  else
  {
    int absolute_index = 0;

    if(!n_absolute_index->has_path("value"))
    {
      ASCENT_ERROR(
          "History: internal error. absolute index does not have child value");
    }
    absolute_index = (*n_absolute_index)["value"].to_int32();

    if(absolute_index >= entries)
    {
      ASCENT_ERROR("History: found only " << entries
                                          << " entries, cannot get entry at "
                                          << absolute_index);
    }
    if(absolute_index < 0)
    {
      ASCENT_ERROR("History: absolute_index must be a non-negative integer.");
    }

    (*output) = history.child(absolute_index);
  }

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprHistoryRange
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistoryRange::ExprHistoryRange()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistoryRange::~ExprHistoryRange()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistoryRange::declare_interface(Node &i)
{
  i["type_name"] = "expr_history_range";
  i["port_names"].append() = "expr_name";
  i["port_names"].append() = "first_absolute_index";
  i["port_names"].append() = "last_absolute_index";
  i["port_names"].append() = "first_relative_index";
  i["port_names"].append() = "last_relative_index";
  i["port_names"].append() = "first_absolute_time";
  i["port_names"].append() = "last_absolute_time";
  i["port_names"].append() = "first_absolute_cycle";
  i["port_names"].append() = "last_absolute_cycle";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistoryRange::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}


//-----------------------------------------------------------------------------
void
ExprHistoryRange::execute()
{
  const string operator_name = "HistoryRange";
  const std::string expr_name  = (*input<conduit::Node>("expr_name"))["name"].as_string();

  const conduit::Node *const cache =
      graph().workspace().registry().fetch<conduit::Node>("cache");

  if(!cache->has_path(expr_name))
  {
    ASCENT_ERROR(operator_name + ": unknown identifier "<<  expr_name);
  }
  const conduit::Node &history = (*cache)[expr_name];

  const conduit::Node *n_first_absolute_index = input<conduit::Node>("first_absolute_index");
  const conduit::Node *n_last_absolute_index  = input<conduit::Node>("last_absolute_index");
  const conduit::Node *n_first_relative_index = input<conduit::Node>("first_relative_index");
  const conduit::Node *n_last_relative_index  = input<conduit::Node>("last_relative_index");
  const conduit::Node *n_first_absolute_time  = input<conduit::Node>("first_absolute_time");
  const conduit::Node *n_last_absolute_time   = input<conduit::Node>("last_absolute_time");
  const conduit::Node *n_first_absolute_cycle = input<conduit::Node>("first_absolute_cycle");
  const conduit::Node *n_last_absolute_cycle  = input<conduit::Node>("last_absolute_cycle");

  conduit::Node *output = detail::range_values_helper(history,
                                                      n_first_absolute_index,
                                                      n_last_absolute_index,
                                                      n_first_relative_index,
                                                      n_last_relative_index,
                                                      n_first_absolute_time,
                                                      n_last_absolute_time,
                                                      n_first_absolute_cycle,
                                                      n_last_absolute_cycle,
                                                      operator_name);

  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprHistoryGradient
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistoryGradient::ExprHistoryGradient()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistoryGradient::~ExprHistoryGradient()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistoryGradient::declare_interface(Node &i)
{
  i["type_name"] = "expr_history_gradient";
  i["port_names"].append() = "expr_name";
  i["port_names"].append() = "window_length";
  i["port_names"].append() = "window_length_unit";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistoryGradient::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistoryGradient::execute()
{
  conduit::Node *output = new conduit::Node();
  const std::string expr_name  = (*input<Node>("expr_name"))["name"].as_string();
  conduit::Node &n_window_length = *input<Node>("window_length");
  conduit::Node &n_window_length_unit = *input<Node>("window_length_unit");

  const conduit::Node *const cache =
      graph().workspace().registry().fetch<Node>("cache");

  if(!cache->has_path(expr_name))
  {
    ASCENT_ERROR("ScalarGradient: unknown identifier "<<  expr_name);
  }

  // handle the optional inputs
  double window_length = 1;
  if(!n_window_length.dtype().is_empty())
  {
    window_length = n_window_length["value"].to_float64();
  }
  if(window_length < 0)
  {
     ASCENT_ERROR("ScalarGradient: window_length must non-negative." );
  }

  string units = "index";
  if(!n_window_length_unit.dtype().is_empty())
  {
    units = n_window_length_unit["value"].as_string();
  }

  bool execution_points = units == "index";
  bool time = units == "time";
  bool cycles = units == "cycle";
  int total = execution_points + time + cycles;

  if(total == 0 && !n_window_length_unit.dtype().is_empty())
  {
     ASCENT_ERROR("HistoryGradient: if a ``window_length_unit`` value is provided,"
                  <<" it must be set to either: 1). \"index\", 2). \"time\", or 3). \"cycle\"." );
  }

  if((execution_points || cycles) && window_length < 1) {
     ASCENT_ERROR("HistoryGradient: window_length must be at least 1 if the window length unit is \"index\" or \"cycle\"." );
  }

  const conduit::Node &history = (*cache)[expr_name];

  const int entries = history.number_of_children();
  if(entries < 2)
  {
    (*output)["value"] = -std::numeric_limits<double>::infinity();
    (*output)["type"] = "double";
    set_output<conduit::Node>(output);
    return;
  }

  int first_index = 0, current_index = entries - 1;
  if(execution_points)
  {
    //clamp the first index if the window length has gone too far
    if(window_length - current_index > 0)
    {
      first_index = 0;
      window_length = current_index;
    }
    else
    {
      first_index = current_index - window_length;
    }
  }
  else if(time)
  {
   string time_path = "time";
   if(!history.child(current_index).has_path(time_path))
    {
      ASCENT_ERROR("HistoryGradient: internal error. current time point does not have the child " + time_path);
    }
    const double current_time = history.child(current_index)[time_path].to_float64();
    const double first_time = current_time - window_length;
    double time;
    for(int index = 0; index < entries; index++)
    {
      if(history.child(index).has_path(time_path))
      {
        time = history.child(index)[time_path].to_float64();
      }
      else
      {
        ASCENT_ERROR("HistoryGradient: a time point in evaluation window (for the calculation at absolute index: " + to_string(index) + ") does not have the child " + time_path );
      }
      if(time >= first_time) {
        first_index = index;
        //adjust so our window length is accurate (since we may not have performed a calculation at precisely the requested time)
        window_length = current_time - time;
        break;
      }
    }
  }
  else if(cycles)
  {
    vector<string> child_names = history.child_names();
    if(child_names.size() != entries)
    {
      ASCENT_ERROR("HistoryGradient: internal error. number of history "
                   <<"entries: " << to_string(entries)
                   <<", but number of history child names: "
                   <<to_string(child_names.size()));
    }
    const unsigned long long current_cycle = stoull(child_names[current_index]);
    const unsigned long long first_cycle = current_cycle - window_length;

    unsigned long long cycle;
    for(int index = 0; index < entries; index++)
    {
      cycle = stoull(child_names[index]);
      if(cycle >= first_cycle)
      {
        first_index = index;
        //adjust so our window length is accurate (since we may not have performed a calculation at precisely the requested time)
        window_length = current_cycle - cycle;
        break;
      }
    }
  }

  string value_path = "";
  vector<string> value_paths = {"value", "attrs/value/value"};
  if(current_index < 0 || current_index >= entries)
  {
    ASCENT_ERROR("HistoryGradient: bad current index: "<<current_index);
  }
  for(const string &path : value_paths)
  {
    if(history.child(current_index).has_path(path))
    {
      value_path = path;
      break;
    }
  }

  if(value_path.size() == 0)
  {
    ASCENT_ERROR("HistoryGradient: internal error. current index does not "
                  <<"have one of the expected value paths");
  }

  if(first_index < 0 || first_index >= entries)
  {
    ASCENT_ERROR("HistoryGradient: bad first index: "<<first_index);
  }

  double first_value = history.child(first_index)[value_path].to_float64();
  double current_value = history.child(current_index)[value_path].to_float64();

  // dy / dx
  double gradient = (current_value - first_value) / window_length;

  (*output)["value"] = gradient;
  (*output)["type"] = "double";

  set_output<conduit::Node>(output);
}


//-----------------------------------------------------------------------------
ExprHistoryGradientRange::ExprHistoryGradientRange()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistoryGradientRange::~ExprHistoryGradientRange()
{
  // empty
}


// -----------------------------------------------------------------------------
void
ExprHistoryGradientRange::declare_interface(Node &i)
{
  i["type_name"] = "expr_history_gradient_range";
  i["port_names"].append() = "expr_name";
  i["port_names"].append() = "first_absolute_index";
  i["port_names"].append() = "last_absolute_index";
  i["port_names"].append() = "first_relative_index";
  i["port_names"].append() = "last_relative_index";
  i["port_names"].append() = "first_absolute_time";
  i["port_names"].append() = "last_absolute_time";
  i["port_names"].append() = "first_absolute_cycle";
  i["port_names"].append() = "last_absolute_cycle";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistoryGradientRange::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}


//-----------------------------------------------------------------------------
void
ExprHistoryGradientRange::execute()
{
  string operator_name = "HistoryGradientRange";

  const std::string expr_name  = (*input<conduit::Node>("expr_name"))["name"].as_string();

  const conduit::Node *const cache =
      graph().workspace().registry().fetch<conduit::Node>("cache");

  if(!cache->has_path(expr_name))
  {
    ASCENT_ERROR(operator_name + ": unknown identifier "<<  expr_name);
  }
  const conduit::Node &history = (*cache)[expr_name];

  const conduit::Node *n_first_absolute_index = input<conduit::Node>("first_absolute_index");
  const conduit::Node *n_last_absolute_index = input<conduit::Node>("last_absolute_index");
  const conduit::Node *n_first_relative_index = input<conduit::Node>("first_relative_index");
  const conduit::Node *n_last_relative_index = input<conduit::Node>("last_relative_index");
  const conduit::Node *n_first_absolute_time = input<conduit::Node>("first_absolute_time");
  const conduit::Node *n_last_absolute_time = input<conduit::Node>("last_absolute_time");
  const conduit::Node *n_first_absolute_cycle = input<conduit::Node>("first_absolute_cycle");
  const conduit::Node *n_last_absolute_cycle = input<conduit::Node>("last_absolute_cycle");

  conduit::Node *output = detail::range_values_helper(history,
                                                      n_first_absolute_index,
                                                      n_last_absolute_index,
                                                      n_first_relative_index,
                                                      n_last_relative_index,
                                                      n_first_absolute_time,
                                                      n_last_absolute_time,
                                                      n_first_absolute_cycle,
                                                      n_last_absolute_cycle,
                                                      operator_name,
                                                      "time");

  size_t num_array_elems = (*output)["value"].dtype().number_of_elements();

  if(num_array_elems < 2)
  {
    double neg_inf[1] = {-std::numeric_limits<double>::infinity()};
    (*output)["value"].set(neg_inf,1);
    (*output)["type"] = "array";
    set_output<conduit::Node>(output);
    return;
  }

  conduit::Node gradient = history_gradient_range((*output)["value"], (*output)["time"]);


  (*output)["value"] = gradient["value"];
  (*output)["type"] = "array";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Histogram Operations
// histogram, entropy, pdf, cdf, quantile, bin_by_value, bin_by_index
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprHistogram
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistogram::ExprHistogram()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistogram::~ExprHistogram()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistogram::declare_interface(Node &i)
{
  i["type_name"] = "expr_histogram";
  i["port_names"].append() = "arg1";
  i["port_names"].append() = "num_bins";
  i["port_names"].append() = "min_val";
  i["port_names"].append() = "max_val";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistogram::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistogram::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");
  // optional inputs
  const conduit::Node *n_bins = input<Node>("num_bins");
  const conduit::Node *n_max = input<Node>("max_val");
  const conduit::Node *n_min = input<Node>("min_val");

  const std::string field = (*arg1)["value"].as_string();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  if(!is_scalar_field(*dataset, field))
  {
    ASCENT_ERROR("Histogram: axis for histogram must be a scalar field. "
                 "Invalid axis field: '"
                 << field << "'.");
  }

  // handle the optional inputs
  int num_bins = 256;
  if(!n_bins->dtype().is_empty())
  {
    num_bins = (*n_bins)["value"].as_int32();
  }

  double min_val;
  double max_val;

  if(!n_max->dtype().is_empty())
  {
    max_val = (*n_max)["value"].to_float64();
  }
  else
  {
    max_val = field_max(*dataset, field)["value"].to_float64();
  }

  if(!n_min->dtype().is_empty())
  {
    min_val = (*n_min)["value"].to_float64();
  }
  else
  {
    min_val = field_min(*dataset, field)["value"].to_float64();
  }

  if(min_val >= max_val)
  {
    ASCENT_ERROR("Histogram: min value ("
                 << min_val << ") must be smaller than max (" << max_val
                 << ")");
  }

  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "histogram";
  (*output)["attrs/value/value"] =
      field_histogram(*dataset, field, min_val, max_val, num_bins)["value"];
  (*output)["attrs/value/type"] = "array";
  (*output)["attrs/min_val/value"] = min_val;
  (*output)["attrs/min_val/type"] = "double";
  (*output)["attrs/max_val/value"] = max_val;
  (*output)["attrs/max_val/type"] = "double";
  (*output)["attrs/num_bins/value"] = num_bins;
  (*output)["attrs/num_bins/type"] = "int";
  (*output)["attrs/clamp/value"] = true;
  (*output)["attrs/clamp/type"] = "bool";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprHistogramEntropy
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistogramEntropy::ExprHistogramEntropy()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistogramEntropy::~ExprHistogramEntropy()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistogramEntropy::declare_interface(Node &i)
{
  i["type_name"] = "expr_histogram_entropy";
  i["port_names"].append() = "hist";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistogramEntropy::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistogramEntropy::execute()
{
  const conduit::Node *hist = input<conduit::Node>("hist");

  if((*hist)["type"].as_string() != "histogram")
  {
    ASCENT_ERROR("Entropy: hist must be a histogram");
  }

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = histogram_entropy(*hist)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprHistogramPDF
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistogramPDF::ExprHistogramPDF() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistogramPDF::~ExprHistogramPDF()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistogramPDF::declare_interface(Node &i)
{
  i["type_name"] = "expr_histogram_pdf";
  i["port_names"].append() = "hist";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistogramPDF::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistogramPDF::execute()
{
  const conduit::Node *hist = input<conduit::Node>("hist");

  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "histogram";
  (*output)["attrs/value/value"] = histogram_pdf(*hist)["value"];
  (*output)["attrs/value/type"] = "array";
  (*output)["attrs/min_val"] = (*hist)["attrs/min_val"];
  (*output)["attrs/max_val"] = (*hist)["attrs/max_val"];
  (*output)["attrs/num_bins"] = (*hist)["attrs/num_bins"];

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprHistogramCDF
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistogramCDF::ExprHistogramCDF() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistogramCDF::~ExprHistogramCDF()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistogramCDF::declare_interface(Node &i)
{
  i["type_name"] = "expr_histogram_cdf";
  i["port_names"].append() = "hist";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistogramCDF::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistogramCDF::execute()
{
  const conduit::Node *hist = input<conduit::Node>("hist");

  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "histogram";
  (*output)["attrs/value/value"] = histogram_cdf(*hist)["value"];
  (*output)["attrs/value/type"] = "array";
  (*output)["attrs/min_val"] = (*hist)["attrs/min_val"];
  (*output)["attrs/max_val"] = (*hist)["attrs/max_val"];
  (*output)["attrs/num_bins"] = (*hist)["attrs/num_bins"];

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprHistogramCDFQuantile
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistogramCDFQuantile::ExprHistogramCDFQuantile()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistogramCDFQuantile::~ExprHistogramCDFQuantile()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistogramCDFQuantile::declare_interface(Node &i)
{
  i["type_name"] = "expr_histogram_cdf_quantile";
  i["port_names"].append() = "cdf";
  i["port_names"].append() = "q";
  i["port_names"].append() = "interpolation";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistogramCDFQuantile::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistogramCDFQuantile::execute()
{
  const conduit::Node *n_cdf = input<conduit::Node>("cdf");
  const conduit::Node *n_val = input<conduit::Node>("q");
  // optional inputs
  const conduit::Node *n_interpolation = input<conduit::Node>("interpolation");

  const double val = (*n_val)["value"].as_float64();

  if(val < 0 || val > 1)
  {
    ASCENT_ERROR("Quantile: val must be between 0 and 1");
  }

  // handle the optional inputs
  std::string interpolation = "linear";
  if(!n_interpolation->dtype().is_empty())
  {
    interpolation = (*n_interpolation)["value"].as_string();
    if(interpolation != "linear" && interpolation != "lower" &&
       interpolation != "higher" && interpolation != "midpoint" &&
       interpolation != "nearest")
    {
      ASCENT_ERROR("Known interpolation types are: linear, lower, higher, "
                   "midpoint, nearest");
    }
  }

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = quantile(*n_cdf, val, interpolation)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}


//*****************************************************************************
// ExprHistogramBinByIndex
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistogramBinByIndex::ExprHistogramBinByIndex()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistogramBinByIndex::~ExprHistogramBinByIndex()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistogramBinByIndex::declare_interface(Node &i)
{
  i["type_name"] = "expr_histogram_bin_by_index";
  i["port_names"].append() = "hist";
  i["port_names"].append() = "bin";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistogramBinByIndex::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistogramBinByIndex::execute()
{
  const conduit::Node *n_bin = input<conduit::Node>("bin");
  const conduit::Node *n_hist = input<conduit::Node>("hist");

  int num_bins = (*n_hist)["attrs/num_bins/value"].as_int32();
  int bin = (*n_bin)["value"].as_int32();

  if(bin < 0 || bin > num_bins - 1)
  {
    ASCENT_ERROR("BinByIndex: bin index must be within the bounds of hist [0, "
                 << num_bins - 1 << "]");
  }

  conduit::Node *output = new conduit::Node();
  const double *bins = (*n_hist)["attrs/value/value"].value();
  (*output)["value"] = bins[bin];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprHistogramBinByValue
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprHistogramBinByValue::ExprHistogramBinByValue()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprHistogramBinByValue::~ExprHistogramBinByValue()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprHistogramBinByValue::declare_interface(Node &i)
{
  i["type_name"] = "expr_histogram_bin_by_value";
  i["port_names"].append() = "hist";
  i["port_names"].append() = "val";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprHistogramBinByValue::verify_params(const conduit::Node &params,
                                       conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprHistogramBinByValue::execute()
{
  const conduit::Node *n_val = input<conduit::Node>("val");
  const conduit::Node *n_hist = input<conduit::Node>("hist");

  double val = (*n_val)["value"].to_float64();
  double min_val = (*n_hist)["attrs/min_val/value"].to_float64();
  double max_val = (*n_hist)["attrs/max_val/value"].to_float64();
  int num_bins = (*n_hist)["attrs/num_bins/value"].as_int32();

  if(val < min_val || val > max_val)
  {
    ASCENT_ERROR("BinByValue: val must within the bounds of hist ["
                 << min_val << ", " << max_val << "]");
  }

  const double inv_delta = num_bins / (max_val - min_val);
  int bin = static_cast<int>((val - min_val) * inv_delta);

  conduit::Node *output = new conduit::Node();
  const double *bins = (*n_hist)["attrs/value/value"].value();
  (*output)["value"] = bins[bin];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Mesh Operations
// cycle, time, field, topology, bounds, lineout
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprMeshCycle
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshCycle::ExprMeshCycle()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshCycle::~ExprMeshCycle()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshCycle::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_cycle";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshCycle::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshCycle::execute()
{
  conduit::Node *output = new conduit::Node();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  // we are just getting state so we don't care if its high or low
  // order
  const conduit::Node *const dataset = data_object->as_node().get();

  conduit::Node state = get_state_var(*dataset, "cycle");
  if(!state.dtype().is_number())
  {
    ASCENT_ERROR("Expressions: cycle() is not a number");
  }

  (*output)["type"] = "int";
  (*output)["value"] = state;
  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshTime
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshTime::ExprMeshTime() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshTime::~ExprMeshTime()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshTime::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_time";
  i["port_names"] = DataType::empty();
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshTime::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshTime::execute()
{
  conduit::Node *output = new conduit::Node();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  // we are just getting state so we don't care if its high or low
  // order
  const conduit::Node *const dataset = data_object->as_node().get();

  conduit::Node state = get_state_var(*dataset, "time");
  if(!state.dtype().is_number())
  {
    ASCENT_ERROR("Expressions: time() is not a number");
  }

  (*output)["type"] = "double";
  (*output)["value"] = state;
  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshField
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshField::ExprMeshField()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshField::~ExprMeshField()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshField::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_field";
  i["port_names"].append() = "field_name";
  i["port_names"].append() = "component";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshField::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshField::execute()
{
  const conduit::Node *n_field_name = input<Node>("field_name");
  std::string field_name = (*n_field_name)["value"].as_string();

  // optional parameters
  const conduit::Node *n_component = input<Node>("component");

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("Field: Missing dataset");
  }

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  if(!has_field(*dataset, field_name))
  {
    std::string known;
    if(dataset->number_of_children() > 0 )
    {
      std::vector<std::string> names = dataset->child(0)["fields"].child_names();
      std::stringstream ss;
      ss << "[";
      for(size_t i = 0; i < names.size(); ++i)
      {
        ss << " '" << names[i]<<"'";
      }
      ss << "]";
      known = ss.str();
    }
    ASCENT_ERROR("Field: dataset does not contain field '"
                 << field_name << "'"
                 << " known = " << known);
  }

  std::string component;
  if(!n_component->dtype().is_empty())
  {
    component = (*n_component)["value"].as_string();
    if(!has_component(*dataset, field_name, component))
    {
      ASCENT_ERROR("Field variable '"
                   << field_name << "'"
                   << " does not have component '" << component << "'."
                   << " known components = "
                   << possible_components(*dataset, field_name));
    }
  }

  // at this point, we know that the field exists.
  // If the the field has only one component then we
  // don't require that the name be provide, but the
  // code will need the name.


  // if the field only has one component use that
  if(component.empty())
  {

    int num_comps = num_components(*dataset, field_name);
    if(num_comps == 1)
    {
      const int comp_idx = 0;
      component = component_name(*dataset, field_name, comp_idx);
    }
    else if(num_comps == 0)
    {
      // default name for empty path
      component = "";
    }
  }

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = field_name;
  if(!component.empty())
  {
    (*output)["component"] = component;
  }
  (*output)["type"] = "field";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshTopology
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshTopology::ExprMeshTopology() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshTopology::~ExprMeshTopology()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshTopology::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_topology";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshTopology::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshTopology::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");

  const std::string topo = (*arg1)["value"].as_string();

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("Topology: Missing dataset");
  }

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  if(!has_topology(*dataset, topo))
  {
    std::set<std::string> names = topology_names(*dataset);
    std::stringstream msg;
    msg<<"Unknown topology: '"<<topo<<"'. Known topologies: [";
    for(auto &name : names)
    {
      msg<<" "<<name;
    }
    msg<<" ]";
    ASCENT_ERROR(msg.str());
  }

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = topo;
  (*output)["type"] = "topology";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshBounds
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshBounds::ExprMeshBounds()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshBounds::~ExprMeshBounds()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshBounds::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_bounds";
  i["port_names"].append() = "topology";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshBounds::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshBounds::execute()
{
  conduit::Node &n_topology = *input<Node>("topology");
  conduit::Node *output = new conduit::Node();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  std::set<std::string> topos;

  if(!n_topology.dtype().is_empty())
  {
    std::string topo = n_topology["value"].as_string();
    if(!has_topology(*dataset, topo))
    {
      std::set<std::string> names = topology_names(*dataset);
      std::stringstream msg;
      msg<<"Unknown topology: '"<<topo<<"'. Known topologies: [";
      for(auto &name : names)
      {
        msg<<" "<<name;
      }
      msg<<" ]";
      ASCENT_ERROR(msg.str());
    }
    topos.insert(topo);
  }
  else
  {
    topos = topology_names(*dataset);
  }

  double inf = std::numeric_limits<double>::infinity();
  double min_vec[3] = {inf, inf, inf};
  double max_vec[3] = {-inf, -inf, -inf};
  for(auto &topo_name : topos)
  {
    conduit::Node n_aabb = global_bounds(*dataset, topo_name);
    double *t_min = n_aabb["min_coords"].as_float64_ptr();
    double *t_max = n_aabb["max_coords"].as_float64_ptr();
    for(int i = 0; i < 3; ++i)
    {
      min_vec[i] = std::min(t_min[i],min_vec[i]);
      max_vec[i] = std::max(t_max[i],max_vec[i]);
    }
  }

  (*output)["type"] = "aabb";
  (*output)["attrs/min/value"].set(min_vec, 3);
  (*output)["attrs/min/type"] = "vector";
  (*output)["attrs/max/value"].set(max_vec, 3);
  (*output)["attrs/max/type"] = "vector";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshLineout
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshLineout::ExprMeshLineout() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshLineout::~ExprMeshLineout()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshLineout::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_lineout";
  i["port_names"].append() = "samples";
  i["port_names"].append() = "start";
  i["port_names"].append() = "end";
  i["port_names"].append() = "fields";
  i["port_names"].append() = "empty_val";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshLineout::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshLineout::execute()
{

#if defined(ASCENT_DRAY_ENABLED)
  conduit::Node &n_samples = *input<Node>("samples");
  int32 samples = n_samples["value"].to_int32();;
  if(samples < 1)
  {
    ASCENT_ERROR("Lineout: samples must be greater than zero: '"<<samples<<"'\n");
  }

  conduit::Node &n_start = *input<Node>("start");
  double *p_start = n_start["value"].as_float64_ptr();

  dray::Vec<dray::Float,3> start;
  start[0] = static_cast<dray::Float>(p_start[0]);
  start[1] = static_cast<dray::Float>(p_start[1]);
  start[2] = static_cast<dray::Float>(p_start[2]);

  conduit::Node &n_end= *input<Node>("end");
  double *p_end = n_end["value"].as_float64_ptr();

  dray::Vec<dray::Float,3> end;
  end[0] = static_cast<dray::Float>(p_end[0]);
  end[1] = static_cast<dray::Float>(p_end[1]);
  end[2] = static_cast<dray::Float>(p_end[2]);

  conduit::Node *output = new conduit::Node();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  dray::Collection * collection = data_object->as_dray_collection().get();

  dray::Lineout lineout;

  lineout.samples(samples);

  conduit::Node &n_empty_val = *input<Node>("empty_val");
  if(!n_empty_val.dtype().is_empty())
  {
    double empty_val = n_empty_val["value"].to_float64();
    lineout.empty_val(empty_val);
  }

  // figure out the number of fields we will use
  conduit::Node &n_fields = (*input<Node>("fields"))["value"];
  const int num_fields = n_fields.number_of_children();
  if(num_fields > 0)
  {
    for(int i = 0; i < num_fields; ++i)
    {
      const conduit::Node &n_field = n_fields.child(i);
      if(n_field["type"].as_string() != "string")
      {
        ASCENT_ERROR("Lineout: field list item is not a string");
      }

      lineout.add_var(n_field["value"].as_string());
    }
  }
  else
  {
    std::set<std::string> field_names;
    // use all fields
    for(int i = 0; i < collection->size(); ++i)
    {
      dray::DataSet dset = collection->domain(i);
      std::vector<std::string> d_names = dset.fields();
      for(int n = 0; n < d_names.size(); ++n)
      {
        field_names.insert(d_names[n]);
      }
    }
    gather_strings(field_names);
  }

  lineout.add_line(start, end);

  dray::Lineout::Result res = lineout.execute(*collection);
  (*output)["type"] = "lineout";
  (*output)["attrs/empty_value/value"] = double(res.m_empty_val);
  (*output)["attrs/empty_value/type"] = "double";
  (*output)["attrs/samples/value"] = int(res.m_points_per_line);
  (*output)["attrs/samples/type"] = "int";
  // we only have one line so the size of points is the size of everything
  const int size = res.m_points.size();
  (*output)["attrs/coordinates/x/value"] = conduit::DataType::float64(size);
  (*output)["attrs/coordinates/x/type"] = "array";
  (*output)["attrs/coordinates/y/value"] = conduit::DataType::float64(size);
  (*output)["attrs/coordinates/y/type"] = "array";
  (*output)["attrs/coordinates/z/value"] = conduit::DataType::float64(size);
  (*output)["attrs/coordinates/z/type"] = "array";
  float64_array x = (*output)["attrs/coordinates/x/value"].value();
  float64_array y = (*output)["attrs/coordinates/y/value"].value();
  float64_array z = (*output)["attrs/coordinates/z/value"].value();
  for(int i = 0; i < size; ++i)
  {
    dray::Vec<dray::Float,3> p = res.m_points.get_value(i);
    x[i] = static_cast<double>(p[0]);
    y[i] = static_cast<double>(p[1]);
    z[i] = static_cast<double>(p[2]);
  }

  const int var_size = res.m_vars.size();
  for(int v = 0; v < var_size; ++v)
  {
    std::string var = res.m_vars[v];
    (*output)["attrs/vars/"+var+"/value"] = conduit::DataType::float64(size);
    (*output)["attrs/vars/"+var+"/type"] = "array";
    float64_array var_array = (*output)["attrs/vars/"+var+"/value"].value();
    for(int i = 0; i < size; ++i)
    {
      var_array[i] = static_cast<double>(res.m_values[v].get_value(i));
    }
  }

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
  
#else
  ASCENT_ERROR("Lineout only supported when Devil Ray is built");
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Mesh Field Operations
//  min reduce, max reduce, avg reduce, sum reduce, nan count, inf count
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprMeshFieldReductionMin
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshFieldReductionMin::ExprMeshFieldReductionMin()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshFieldReductionMin::~ExprMeshFieldReductionMin()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionMin::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_field_reduction_min";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshFieldReductionMin::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionMin::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *output = new conduit::Node();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  if(!is_scalar_field(*dataset, field))
  {
    ASCENT_ERROR("ExprFieldReductionMin: field '"
                 << field << "' is not a scalar field");
  }

  // TODO
  conduit::Node n_min = field_min(*dataset, field);

  (*output)["type"] = "value_position";
  (*output)["attrs/value/value"] = n_min["value"];
  (*output)["attrs/value/type"] = "double";
  (*output)["attrs/position/value"] = n_min["position"];
  (*output)["attrs/position/type"] = "vector";
  // information about the element/field
  (*output)["attrs/element/rank"] = n_min["rank"];
  (*output)["attrs/element/domain_index"] = n_min["domain_id"];
  (*output)["attrs/element/index"] = n_min["index"];
  (*output)["attrs/element/assoc"] = n_min["assoc"];

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshFieldReductionMax
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshFieldReductionMax::ExprMeshFieldReductionMax()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshFieldReductionMax::~ExprMeshFieldReductionMax()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionMax::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_field_reduction_max";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshFieldReductionMax::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionMax::execute()
{

  const conduit::Node *arg1 = input<Node>("arg1");

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *output = new conduit::Node();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  if(!is_scalar_field(*dataset, field))
  {
    ASCENT_ERROR("FieldMax: field '" << field << "' is not a scalar field");
  }

  conduit::Node n_max = field_max(*dataset, field);

  (*output)["type"] = "value_position";
  (*output)["attrs/value/value"] = n_max["value"];
  (*output)["attrs/value/type"] = "double";
  (*output)["attrs/position/value"] = n_max["position"];
  (*output)["attrs/position/type"] = "vector";
  // information about the element/field
  (*output)["attrs/element/rank"] = n_max["rank"];
  (*output)["attrs/element/domain_index"] = n_max["domain_id"];
  (*output)["attrs/element/index"] = n_max["index"];
  (*output)["attrs/element/assoc"] = n_max["assoc"];

  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshFieldReductionAvg
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshFieldReductionAvg::ExprMeshFieldReductionAvg()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshFieldReductionAvg::~ExprMeshFieldReductionAvg()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionAvg::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_field_reduction_avg";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshFieldReductionAvg::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionAvg::execute()
{
  const conduit::Node *arg1 = input<Node>("arg1");

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *output = new conduit::Node();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  if(!is_scalar_field(*dataset, field))
  {
    ASCENT_ERROR("FieldAvg: field '" << field << "' is not a scalar field");
  }

  conduit::Node n_avg = field_avg(*dataset, field);

  (*output)["value"] = n_avg["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshFieldReductionSum
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshFieldReductionSum::ExprMeshFieldReductionSum() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshFieldReductionSum::~ExprMeshFieldReductionSum()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionSum::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_field_reduction_sum";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshFieldReductionSum::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionSum::execute()
{
  std::string field = (*input<Node>("arg1"))["value"].as_string();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = field_sum(*dataset, field)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshFieldReductionNanCount
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshFieldReductionNanCount::ExprMeshFieldReductionNanCount() : Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshFieldReductionNanCount::~ExprMeshFieldReductionNanCount()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionNanCount::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_field_reduction_nan_count";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshFieldReductionNanCount::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionNanCount::execute()
{
  std::string field = (*input<Node>("arg1"))["value"].as_string();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  conduit::Node *dataset = data_object->as_low_order_bp().get();

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = field_nan_count(*dataset, field)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshFieldReductionInfCount
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshFieldReductionInfCount::ExprMeshFieldReductionInfCount()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshFieldReductionInfCount::~ExprMeshFieldReductionInfCount()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionInfCount::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_field_reduction_inf_count";
  i["port_names"].append() = "arg1";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshFieldReductionInfCount::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshFieldReductionInfCount::execute()
{
  std::string field = (*input<Node>("arg1"))["value"].as_string();

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  conduit::Node *dataset = data_object->as_low_order_bp().get();

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = field_inf_count(*dataset, field)["value"];
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Binning Operations
//  binning, binning_axis, bin_by_index, point_and_axis, max_from_point
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
// ExprMeshBinning
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshBinning::ExprMeshBinning()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshBinning::~ExprMeshBinning()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshBinning::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_binning";
  i["port_names"].append() = "reduction_var";
  i["port_names"].append() = "reduction_op";
  i["port_names"].append() = "bin_axes";
  i["port_names"].append() = "empty_bin_val";
  i["port_names"].append() = "component";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshBinning::verify_params(const conduit::Node &params, conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshBinning::execute()
{
  // why isn't this an input?
  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");

  conduit::Node *dataset = data_object->as_low_order_bp().get();

  const std::string reduction_var =
      (*input<Node>("reduction_var"))["value"].as_string();
  const std::string reduction_op =
      (*input<Node>("reduction_op"))["value"].as_string();
  const conduit::Node *n_axes_list = input<Node>("bin_axes");
  // optional arguments
  const conduit::Node *n_empty_bin_val = input<conduit::Node>("empty_bin_val");
  const conduit::Node *n_component = input<conduit::Node>("component");

  conduit::Node n_binning;
  conduit::Node n_bin_axes;

  binning_interface(reduction_var,
                    reduction_op,
                    *n_empty_bin_val,
                    *n_component,
                    *n_axes_list,
                    *dataset,
                    n_binning,
                    n_bin_axes);

  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "binning";
  (*output)["attrs/value/value"] = n_binning["value"];
  (*output)["attrs/value/type"] = "array";
  (*output)["attrs/reduction_var/value"] = reduction_var;
  (*output)["attrs/reduction_var/type"] = "string";
  (*output)["attrs/reduction_op/value"] = reduction_op;
  (*output)["attrs/reduction_op/type"] = "string";
  (*output)["attrs/bin_axes/value"] = n_bin_axes;
  //(*output)["attrs/bin_axes/type"] = "list";
  (*output)["attrs/association/value"] = n_binning["association"];
  (*output)["attrs/association/type"] = "string";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);

}

//*****************************************************************************
// ExprMeshBinningAxis
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshBinningAxis::ExprMeshBinningAxis()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshBinningAxis::~ExprMeshBinningAxis()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningAxis::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_binning_axis";
  i["port_names"].append() = "name";
  i["port_names"].append() = "min_val";
  i["port_names"].append() = "max_val";
  i["port_names"].append() = "num_bins";
  i["port_names"].append() = "bins";
  i["port_names"].append() = "clamp";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshBinningAxis::verify_params(const conduit::Node &params,
                                   conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningAxis::execute()
{
  const std::string name = (*input<Node>("name"))["value"].as_string();
  // uniform binning
  const conduit::Node *n_min = input<Node>("min_val");
  const conduit::Node *n_max = input<Node>("max_val");
  const conduit::Node *n_num_bins = input<Node>("num_bins");
  // rectilinear binning
  const conduit::Node *n_bins_list_obj = input<Node>("bins");
  // clamp
  const conduit::Node *n_clamp = input<conduit::Node>("clamp");

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("Axis: Missing dataset");
  }

  DataObject *data_object =
    graph().workspace().registry().fetch<DataObject>("dataset");
  const conduit::Node *const dataset = data_object->as_low_order_bp().get();

  if(!is_scalar_field(*dataset, name) && !is_xyz(name))
  {
    std::string known;
    if(dataset->number_of_children() > 0 )
    {
      std::vector<std::string> names = dataset->child(0)["fields"].child_names();
      std::stringstream ss;
      ss << "[";
      for(size_t i = 0; i < names.size(); ++i)
      {
        ss << " '" << names[i]<<"'";
      }
      ss << "]";
      known = ss.str();
    }

    ASCENT_ERROR("Binning Axis: Axes must be scalar fields or x/y/z. Dataset does not "
                 "contain scalar field '"
                 << name << "'. Possible field names "<<known<<".");
  }

  conduit::Node *output;
  if(!n_bins_list_obj->dtype().is_empty())
  {
    const conduit::Node &n_bins_list = (*n_bins_list_obj)["value"];
    // ensure none of the uniform binning arguments are passed
    if(!n_min->dtype().is_empty() || !n_max->dtype().is_empty() ||
       !n_num_bins->dtype().is_empty())
    {
      ASCENT_ERROR("Binning Axis: Only pass in arguments for uniform or rectilinear "
                   "binning, not both.");
    }

    int bins_len = n_bins_list.number_of_children();

    if(bins_len < 2)
    {
      ASCENT_ERROR("Binning Axis: bins must have at least 2 items.");
    }

    output = new conduit::Node();
    (*output)["value/" + name + "/bins"].set(
        conduit::DataType::c_double(bins_len));
    double *bins = (*output)["value/" + name + "/bins"].value();

    for(int i = 0; i < bins_len; ++i)
    {
      const conduit::Node &bin = n_bins_list.child(i);
      if(!detail::is_scalar(bin["type"].as_string()))
      {
        delete output;
        ASCENT_ERROR("Binning Axis: bins must be a list of scalars.");
      }
      bins[i] = bin["value"].to_float64();
      if(i != 0 && bins[i - 1] >= bins[i])
      {
        delete output;
        ASCENT_ERROR("Binning Axis: bin extents must be strictly increasing scalars.");
      }
    }
  }
  else
  {
    output = new conduit::Node();

    // Normally, we would do all the error checking here/ determined
    // the mins and maxs, but there are some issues. Since we are
    // merely an axis , which is part of data binning, we are not
    // aware of the field that the binning is on. Thus, we can't
    // automatically figure out the spatial min/max since
    // we don't know the topoloy. Thus we will defer the error
    // checking to the actual binning code.
    double min_val;
    bool min_found = false;
    if(!n_min->dtype().is_empty())
    {
      min_val = (*n_min)["value"].to_float64();
      (*output)["value/" + name + "/min_val"] = min_val;
      min_found = true;
    }

    double max_val;
    bool max_found = false;
    if(!n_max->dtype().is_empty())
    {
      max_val = (*n_max)["value"].to_float64();
      max_found = true;
      (*output)["value/" + name + "/max_val"] = max_val;
    }

    // default num bins
    (*output)["value/" + name + "/num_bins"] = 256;

    if(!n_num_bins->dtype().is_empty())
    {
      (*output)["value/" + name + "/num_bins"] =
          (*n_num_bins)["value"].to_int32();
    }

    if(min_found && max_found && min_val >= max_val)
    {
      delete output;
      ASCENT_ERROR("Binning Axis: axis with name '"
                   << name << "': min_val (" << min_val
                   << ") must be smaller than max_val (" << max_val << ")");
    }
  }

  (*output)["value/" + name + "/clamp"] = 0;
  if(!n_clamp->dtype().is_empty())
  {
    (*output)["value/" + name + "/clamp"] = (*n_clamp)["value"].to_int32();
  }

  (*output)["value/" + name];
  (*output)["type"] = "axis";
  set_output<conduit::Node>(output);
}


//*****************************************************************************
// ExprMeshBinningBinByIndex
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshBinningBinByIndex::ExprMeshBinningBinByIndex()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshBinningBinByIndex::~ExprMeshBinningBinByIndex()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningBinByIndex::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_binning_bin_by_index";
  i["port_names"].append() = "binning";
  i["port_names"].append() = "index";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshBinningBinByIndex::verify_params(const conduit::Node &params,
                                         conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningBinByIndex::execute()
{
  conduit::Node &in_binning = *input<Node>("binning");
  conduit::Node &in_index =  *input<Node>("index");
  conduit::Node *output = new conduit::Node();

  const int num_axes = in_binning["attrs/bin_axes"].number_of_children();
  if(num_axes > 1)
  {
    ASCENT_ERROR("bin: only one axis is implemented");
  }

  int bindex = in_index["value"].to_int32();

  const conduit::Node &axis = in_binning["attrs/bin_axes/value"].child(0);
  const int num_bins = axis["num_bins"].to_int32();

  if(bindex < 0 || bindex >= num_bins)
  {
    ASCENT_ERROR("bin: invalid bin "<<bindex<<"."
                <<" Number of bins "<<num_bins);
  }

  const double min_val = axis["min_val"].to_float64();
  const double max_val = axis["max_val"].to_float64();
  const double bin_size = (max_val - min_val) / double(num_bins);
  double *bins = in_binning["attrs/value/value"].value();

  double left = min_val + double(bindex) * bin_size;
  double right = min_val + double(bindex+1) * bin_size;
  double center = left + (right-left) / 2.0;
  double val = bins[bindex];

  (*output)["type"] = "bin";

  (*output)["attrs/value/value"] = val;
  (*output)["attrs/value/type"] = "double";

  (*output)["attrs/min/value"] = left;
  (*output)["attrs/min/type"] = "double";

  (*output)["attrs/max/value"] = right;
  (*output)["attrs/max/type"] = "double";

  (*output)["attrs/center/value"] = center;
  (*output)["attrs/center/type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshBinningPointAndAxis
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshBinningPointAndAxis::ExprMeshBinningPointAndAxis()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshBinningPointAndAxis::~ExprMeshBinningPointAndAxis()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningPointAndAxis::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_binning_point_and_axis";
  i["port_names"].append() = "binning";
  i["port_names"].append() = "axis";
  i["port_names"].append() = "threshold";
  i["port_names"].append() = "point";
  i["port_names"].append() = "miss_value";
  i["port_names"].append() = "direction";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshBinningPointAndAxis::verify_params(const conduit::Node &params,
                                           conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningPointAndAxis::execute()
{
  conduit::Node &in_binning = *input<Node>("binning");
  conduit::Node &in_axis =  *input<Node>("axis");
  conduit::Node &in_threshold =  *input<Node>("threshold");
  conduit::Node &in_point =  *input<Node>("point");
  conduit::Node &n_miss_val =  *input<Node>("miss_value");
  conduit::Node &n_dir =  *input<Node>("direction");
  conduit::Node *output = new conduit::Node();

  const int num_axes = in_binning["attrs/bin_axes"].number_of_children();
  if(num_axes > 1)
  {
    ASCENT_ERROR("point_and_axis: only one axis is implemented");
  }

  int direction = 1;
  if(!n_dir.dtype().is_empty())
  {
    direction = n_dir["value"].to_int32();
    if(direction != 1 && direction != -1)
    {
      ASCENT_ERROR("point_and_axis: invalid direction `"<<direction<<"'."
                  <<" Valid directions are 1 or -1.");
    }
  }

  const double point = in_point["value"].to_float64();
  const double threshold = in_threshold["value"].to_float64();

  const conduit::Node &axis = in_binning["attrs/bin_axes/value"].child(0);
  const int num_bins = axis["num_bins"].to_int32();
  const double min_val = axis["min_val"].to_float64();
  const double max_val = axis["max_val"].to_float64();
  const double bin_size = (max_val - min_val) / double(num_bins);

  double *bins = in_binning["attrs/value/value"].value();
  double min_dist = std::numeric_limits<double>::max();
  int index = -1;
  for(int i = 0; i < num_bins; ++i)
  {
    double val = bins[i];
    if(val > threshold)
    {
      double left = min_val + double(i) * bin_size;
      double right = min_val + double(i+1) * bin_size;
      double center = left + (right-left) / 2.0;
      double dist = center - point;
      // skip if distance is behind
      bool behind = dist * double(direction) < 0;

      if(!behind && dist < min_dist)
      {
        min_dist = dist;
        index = i;
      }
    }
  }

  double bin_value = std::numeric_limits<double>::quiet_NaN();

  if(!n_miss_val.dtype().is_empty())
  {
    bin_value = n_miss_val["value"].to_float64();
  }

  // init with miss
  double bin_min = bin_value;
  double bin_max = bin_value;
  double bin_center = bin_value;

  if(index != -1)
  {
    bin_value = bins[index];
    bin_min = min_val + double(index) * bin_size;
    bin_max = min_val + double(index+1) * bin_size;
    bin_center = bin_min + (bin_max-bin_min) / 2.0;
  }

  (*output)["type"] = "bin";

  (*output)["attrs/value/value"] = bin_value;
  (*output)["attrs/value/type"] = "double";

  (*output)["attrs/min/value"] = bin_min;
  (*output)["attrs/min/type"] = "double";

  (*output)["attrs/max/value"] = bin_max;
  (*output)["attrs/max/type"] = "double";

  (*output)["attrs/center/value"] = bin_center;
  (*output)["attrs/center/type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}

//*****************************************************************************
// ExprMeshBinningMaxFromPoint
//*****************************************************************************

//-----------------------------------------------------------------------------
ExprMeshBinningMaxFromPoint::ExprMeshBinningMaxFromPoint()
: Filter()
{
  // empty
}

//-----------------------------------------------------------------------------
ExprMeshBinningMaxFromPoint::~ExprMeshBinningMaxFromPoint()
{
  // empty
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningMaxFromPoint::declare_interface(Node &i)
{
  i["type_name"] = "expr_mesh_binning_max_from_point";
  i["port_names"].append() = "binning";
  i["port_names"].append() = "axis";
  i["port_names"].append() = "point";
  i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ExprMeshBinningMaxFromPoint::verify_params(const conduit::Node &params,
                                           conduit::Node &info)
{
  info.reset();
  bool res = true;
  return res;
}

//-----------------------------------------------------------------------------
void
ExprMeshBinningMaxFromPoint::execute()
{
  conduit::Node &in_binning = *input<Node>("binning");
  conduit::Node &in_axis    = *input<Node>("axis");
  conduit::Node &in_point   = *input<Node>("point");
  conduit::Node *output     = new conduit::Node();

  const int num_axes = in_binning["attrs/bin_axes"].number_of_children();
  if(num_axes > 1)
  {
    ASCENT_ERROR("max_from_point: only one axis is implemented");
  }

  const double point = in_point["value"].to_float64();

  const conduit::Node &axis = in_binning["attrs/bin_axes/value"].child(0);
  const int num_bins = axis["num_bins"].to_int32();
  const double min_val = axis["min_val"].to_float64();
  const double max_val = axis["max_val"].to_float64();
  const double bin_size = (max_val - min_val) / double(num_bins);

  double *bins = in_binning["attrs/value/value"].value();
  double max_bin_val = std::numeric_limits<double>::lowest();
  double dist_value = 0;
  double min_dist = std::numeric_limits<double>::max();
  int index = -1;
  for(int i = 0; i < num_bins; ++i)
  {
    double val = bins[i];
    if(val >= max_bin_val)
    {
      double left = min_val + double(i) * bin_size;
      double right = min_val + double(i+1) * bin_size;
      double center = left + (right-left) / 2.0;
      double dist = fabs(center - point);
      if(val > max_bin_val ||
         ((dist < min_dist) && val == max_bin_val))
      {
        min_dist = dist;
        max_bin_val = val;
        dist_value = center - point;
        index = i;
      }
    }
  }

  double loc[3] = {0.0, 0.0, 0.0};
  std::string axis_str = in_axis["value"].as_string();

  if(axis_str == "z")
  {
    loc[2] = dist_value;
  }
  else if (axis_str == "y")
  {
    loc[1] = dist_value;
  }
  else
  {
    loc[0] = dist_value;
  }

  (*output)["type"] = "value_position";
  (*output)["attrs/value/value"] = max_bin_val;
  (*output)["attrs/value/type"] = "double";
  (*output)["attrs/position/value"].set(loc,3);
  (*output)["attrs/position/type"] = "vector";

  (*output)["value"] = min_dist;
  (*output)["type"] = "double";

  resolve_symbol_result(graph(), output, this->name());
  set_output<conduit::Node>(output);
}


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime::expressions --
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime --
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent:: --
//-----------------------------------------------------------------------------

