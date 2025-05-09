//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
// Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
// other details. No copyright assignment is required to contribute to Ascent.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//-----------------------------------------------------------------------------
///
/// file: ascent_mpi_render_2d.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <ascent.hpp>

#include <iostream>
#include <math.h>
#include <mpi.h>

#include <conduit_blueprint.hpp>

#include "t_config.hpp"
#include "t_utils.hpp"

using namespace std;
using namespace conduit;
using ascent::Ascent;

//-----------------------------------------------------------------------------
TEST(ascent_mpi_runtime, test_render_mpi_2d_main_runtime)
{

    // the vtkm runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent vtkm support disabled, skipping test");
        return;
    }

    //
    // Set Up MPI
    //
    int par_rank;
    int par_size;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &par_rank);
    MPI_Comm_size(comm, &par_size);

    ASCENT_INFO("Rank "
                  << par_rank
                  << " of "
                  << par_size
                  << " reporting");
    //
    // Create the data.
    //
    Node data, verify_info;
    create_2d_example_dataset(data,par_rank,par_size);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    verify_info.print();

    // make sure the _output dir exists
    string output_path = "";
    if(par_rank == 0)
    {
        output_path = prepare_output_dir();
    }
    else
    {
        output_path = output_dir();
    }

    string output_file = conduit::utils::join_file_path(output_path,"tout_render_mpi_2d_default_runtime");

    // remove old images before rendering
    remove_test_image(output_file);

    //
    // Create the actions.
    //

    conduit::Node scenes;
    scenes["s1/plots/p1/type"]         = "pseudocolor";
    scenes["s1/plots/p1/field"] = "radial_vert";
    scenes["s1/image_prefix"] = output_file;

    conduit::Node actions;
    conduit::Node &add_plots = actions.append();
    add_plots["action"] = "add_scenes";
    add_plots["scenes"] = scenes;

    actions.print();


    //
    // Run Ascent
    //

    Ascent ascent;

    Node ascent_opts;
    // we use the mpi handle provided by the fortran interface
    // since it is simply an integer
    ascent_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    ascent_opts["runtime"] = "ascent";
    ascent.open(ascent_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();

    MPI_Barrier(comm);
    // check that we created an image
    EXPECT_TRUE(check_test_image(output_file));
}

//-----------------------------------------------------------------------------
TEST(ascent_mpi_runtime, test_error_for_mpi_vs_non_mpi)
{
    Ascent ascent;
    Node ascent_opts;
    ascent_opts["exceptions"] = "forward";
    // we throw an error if an mpi_comm is NOT provided to a mpi ver of ascent
    EXPECT_THROW(ascent.open(ascent_opts),conduit::Error);
}

//-----------------------------------------------------------------------------
TEST(ascent_mpi_runtime, test_for_error_reading_actions)
{
    int par_rank;
    int par_size;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &par_rank);
    MPI_Comm_size(comm, &par_size);

    Ascent ascent;
    Node ascent_opts, ascent_actions;
    ascent_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    ascent_opts["actions_file"] = "tin_bad_actions.yaml";
    ascent_opts["exceptions"] = "forward";
    
    if(par_rank == 0)
    {
        std::ofstream ofs("tin_bad_actions.yaml", std::ofstream::out);
        ofs  << ":";
        ofs.close();
    }

    ascent.open(ascent_opts);

    //
    // Create the data.
    //
    Node data, verify_info;
    create_2d_example_dataset(data,par_rank,par_size);
    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));

    ascent.publish(data);

    // all tasks should throw an error
    EXPECT_THROW(ascent.execute(ascent_actions),conduit::Error);


    ascent.close();

}

//-----------------------------------------------------------------------------
TEST(ascent_mpi_runtime, test_mpi_logs)
{
    //
    // Set Up MPI
    //
    int par_rank;
    int par_size;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &par_rank);
    MPI_Comm_size(comm, &par_size);

    // make sure the _output dir exists
    string output_path = "";
    if(par_rank == 0)
    {
        output_path = prepare_output_dir();
    }
    else
    {
        output_path = output_dir();
    }

    string log_base = conduit::utils::join_file_path(output_path,"tout_log_mpi_test");

    //
    // Test cases
    //
    // case "0" creates 1 log per mpi task default name in cwd
    //     ascent_log_out_rank_00000.yaml
    //     ascent_log_out_rank_00001.yaml
    // case "a" creates 1 log per mpi
    //     tout_log_mpi_test_rank_a_00.yaml
    //     tout_log_mpi_test_rank_a_01.yaml
    //
    // case "b" creates log only for root (rank 0)
    //     tout_log_mpi_test_rank_b_00.yaml
    //
    // case "c" creates log only on rank 1
    //     tout_log_mpi_test_rank_c_01.yaml
    //


    if(par_rank == 0)
    {
        conduit::utils::remove_path_if_exists("ascent_log_output_rank_00000.yaml");
        conduit::utils::remove_path_if_exists("ascent_log_output_rank_00001.yaml");
        conduit::utils::remove_path_if_exists(log_base + "_a_00.yaml");
        conduit::utils::remove_path_if_exists(log_base + "_a_01.yaml");
        conduit::utils::remove_path_if_exists(log_base + "_b_00.yaml");
        conduit::utils::remove_path_if_exists(log_base + "_c_01.yaml");
    }

    Ascent ascent;
    // case 0
    Node ascent_opts;
    ascent_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    ascent_opts["logging"] = "true";
    ascent.open(ascent_opts);
    ascent.close();

    // case a
    ascent_opts.reset();
    ascent_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    ascent_opts["logging/file_pattern"] = log_base + "_a_{rank:02d}.yaml";
    ascent.open(ascent_opts);
    ascent.close();

    // case b
    ascent_opts.reset();
    ascent_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    ascent_opts["logging/file_pattern"] = log_base + "_b_{rank:02d}.yaml";
    ascent_opts["logging/ranks"] = "root";
    ascent.open(ascent_opts);
    ascent.close();

    // case c
    ascent_opts.reset();
    ascent_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    ascent_opts["logging/file_pattern"] = log_base + "_c_{rank:02d}.yaml";
    ascent_opts["logging/ranks"] =1;
    ascent.open(ascent_opts);
    ascent.close();

    MPI_Barrier(comm);
    EXPECT_TRUE(conduit::utils::is_file("ascent_log_output_rank_00000.yaml"));
    EXPECT_TRUE(conduit::utils::is_file("ascent_log_output_rank_00001.yaml"));
    EXPECT_TRUE(conduit::utils::is_file(log_base + "_a_00.yaml"));
    EXPECT_TRUE(conduit::utils::is_file(log_base + "_a_01.yaml"));
    EXPECT_TRUE(conduit::utils::is_file(log_base + "_b_00.yaml"));
    EXPECT_TRUE(conduit::utils::is_file(log_base + "_c_01.yaml"));
}

//-----------------------------------------------------------------------------
TEST(ascent_mpi_runtime, test_logging_actions_mpi)
{

    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent vtkm support disabled, skipping test");
        return;
    }

    //
    // Set Up MPI
    //
    int par_rank;
    int par_size;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &par_rank);
    MPI_Comm_size(comm, &par_size);

    //
    // Create an example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                              5,
                                              5,
                                              5,
                                              data);
    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));

    string output_path = "";
    if(par_rank == 0)
    {
        output_path = prepare_output_dir();
    }
    else
    {
        output_path = output_dir();
    }
    string output_file = conduit::utils::join_file_path(output_path,"tout_logging_render4_");
    string log_base = conduit::utils::join_file_path(output_path,"ascent_action_log_mpi_");
    string log_file = log_base +"{rank:05d}.yaml";

    // remove old images/log files before rendering
    if(par_rank == 0)
    {
        remove_test_image(output_file);
        conduit::utils::remove_path_if_exists(log_base + "00000.yaml");
    }

    conduit::Node actions;
    conduit::Node &add_scenes= actions.append();
    add_scenes["action"] = "add_scenes";
    conduit::Node &scenes = add_scenes["scenes"];
    scenes["s1/plots/p1/type"]         = "pseudocolor";
    scenes["s1/plots/p1/field"] = "braid";
    scenes["s1/image_prefix"] = output_file;

    conduit::Node actions_begin_logs;
    conduit::Node &begin_logs= actions_begin_logs.append();
    begin_logs["action"] = "open_log";
    begin_logs["file_pattern"] = log_file;
    begin_logs["log_threshold"] = "all";

    conduit::Node actions_flush_logs;
    conduit::Node &flush_logs= actions_flush_logs.append();
    flush_logs["action"] = "flush_log";

    conduit::Node actions_close_logs;
    conduit::Node &close_logs= actions_close_logs.append();
    close_logs["action"] = "close_log";

    //
    // Run Ascent
    //

    Ascent ascent;

    Node ascent_opts;
    ascent_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    ascent_opts["runtime"] = "ascent";
    ascent.open(ascent_opts);

    ascent.publish(data);
    ascent.execute(actions_begin_logs);
    ascent.execute(actions);
    ascent.execute(actions_flush_logs);
    ascent.execute(actions_close_logs);
    ascent.close();

    // check that the log file exists
    MPI_Barrier(comm);
    EXPECT_TRUE(conduit::utils::is_file(log_base + "00000.yaml"));

    // check that the log file has the expected number of logs in it (1 open, 3 execution, 1 close)
    conduit::Node log_file_contents;
    log_file_contents.load(log_base + "00000.yaml");
    EXPECT_EQ(log_file_contents.number_of_children(), 6);
}


//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    int result = 0;

    ::testing::InitGoogleTest(&argc, argv);
    MPI_Init(&argc, &argv);
    result = RUN_ALL_TESTS();
    MPI_Finalize();

    return result;
}


