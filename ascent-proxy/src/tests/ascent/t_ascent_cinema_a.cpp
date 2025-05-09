//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
// Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
// other details. No copyright assignment is required to contribute to Ascent.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//-----------------------------------------------------------------------------
///
/// file: t_ascent_cinema_a.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <ascent.hpp>

#include <iostream>
#include <math.h>

#include <conduit_blueprint.hpp>

#include "t_config.hpp"
#include "t_utils.hpp"



using namespace std;
using namespace conduit;
using namespace ascent;

index_t EXAMPLE_MESH_SIDE_DIM = 32;

//-----------------------------------------------------------------------------
TEST(ascent_cinema_a, test_cinema_a)
{
    // the vtkm runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent support disabled, skipping test");
        return;
    }

    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    std::string db_name = "test_db1";
    string output_path = "./cinema_databases/" + db_name;
    string output_file = conduit::utils::join_file_path(output_path, "info.json");
    // remove old file before rendering
    if(conduit::utils::is_file(output_file))
    {
        conduit::utils::remove_file(output_file);
    }

    //
    // Create the actions.
    //
    Node actions;

    conduit::Node pipelines;
    // pipeline 1
    pipelines["pl1/f1/type"] = "contour";
    // filter knobs
    conduit::Node &contour_params = pipelines["pl1/f1/params"];
    contour_params["field"] = "braid";
    contour_params["iso_values"] = 0.;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"] = "pseudocolor";
    scenes["scene1/plots/plt1/pipeline"] = "pl1";
    scenes["scene1/plots/plt1/field"] = "braid";
    // setup required cinema params
    scenes["scene1/renders/r1/type"] = "cinema";
    scenes["scene1/renders/r1/phi"] = 2;
    scenes["scene1/renders/r1/theta"] = 2;
    scenes["scene1/renders/r1/db_name"] = "test_db1";
    //scenes["scene1/renders/r1/annotations"] = "false";
    scenes["scene1/renders/r1/camera/zoom"] = 1.0; // no zoom

    // add the pipeline
    conduit::Node &add_pipelines = actions.append();
    add_pipelines["action"] = "add_pipelines";
    add_pipelines["pipelines"] = pipelines;
    // add scene
    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    actions.print();

    //
    // Run Ascent
    //

    Ascent ascent;
    Node ascent_opts;
    // default is now ascent
    ascent_opts["runtime/type"] = "ascent";
    ascent.open(ascent_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();

    // check that we created an image
    EXPECT_TRUE(conduit::utils::is_file(output_file));
}


//-----------------------------------------------------------------------------
TEST(ascent_cinema_a, test_cinema_a_annotations_off)
{
    // the vtkm runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent support disabled, skipping test");
        return;
    }

    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));

    std::string db_no_annot = "test_db1_no_annotations";
    std::string db_world_no_annot  = "test_db1_no_world_annotations";
    std::string db_no_screen_annot = "test_db1_no_screen_annotations";

    string output_path_base = conduit::utils::join_file_path(".","cinema_databases");
    conduit::utils::remove_path_if_exists(conduit::utils::join_file_path(output_path_base, db_no_annot));
    conduit::utils::remove_path_if_exists(conduit::utils::join_file_path(output_path_base, db_world_no_annot));
    conduit::utils::remove_path_if_exists(conduit::utils::join_file_path(output_path_base, db_no_screen_annot));

    Node actions;

    conduit::Node pipelines;
    pipelines["pl1/f1/type"] = "contour";
    conduit::Node &contour_params = pipelines["pl1/f1/params"];
    contour_params["field"] = "braid";
    contour_params["iso_values"] = 0.;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"] = "pseudocolor";
    scenes["scene1/plots/plt1/pipeline"] = "pl1";
    scenes["scene1/plots/plt1/field"] = "braid";

    // no annotations
    scenes["scene1/renders/r1/type"] = "cinema";
    scenes["scene1/renders/r1/phi"] = 2;
    scenes["scene1/renders/r1/theta"] = 2;
    scenes["scene1/renders/r1/db_name"] = db_no_annot;
    scenes["scene1/renders/r1/annotations"] = "false";

    // no world annotations
    scenes["scene1/renders/r2/type"] = "cinema";
    scenes["scene1/renders/r2/phi"] = 2;
    scenes["scene1/renders/r2/theta"] = 2;
    scenes["scene1/renders/r2/db_name"] = db_world_no_annot;
    scenes["scene1/renders/r2/world_annotations"] = "false";

    // no screen annotations
    scenes["scene1/renders/r3/type"] = "cinema";
    scenes["scene1/renders/r3/phi"] = 2;
    scenes["scene1/renders/r3/theta"] = 2;
    scenes["scene1/renders/r3/db_name"] = db_no_screen_annot;
    scenes["scene1/renders/r3/screen_annotations"] = "false";

    // add the pipeline
    conduit::Node &add_pipelines = actions.append();
    add_pipelines["action"] = "add_pipelines";
    add_pipelines["pipelines"] = pipelines;

    // add scene
    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    actions.print();

    // Run Ascent
    Ascent ascent;
    ascent.open();
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();

    // check that we created the db dirs
    EXPECT_TRUE(conduit::utils::is_directory(conduit::utils::join_file_path(output_path_base, db_no_annot)));
    EXPECT_TRUE(conduit::utils::is_directory(conduit::utils::join_file_path(output_path_base, db_world_no_annot)));
    EXPECT_TRUE(conduit::utils::is_directory(conduit::utils::join_file_path(output_path_base, db_no_screen_annot)));
}


//-----------------------------------------------------------------------------
TEST(ascent_cinema_a, test_angle_range_1)
{
    // the vtkm runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent support disabled, skipping test");
        return;
    }

    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    std::string db_name = "test_db2";
    string output_path = "./cinema_databases/" + db_name;
    string output_file = conduit::utils::join_file_path(output_path, "info.json");
    // remove old file before rendering
    if(conduit::utils::is_file(output_file))
    {
        conduit::utils::remove_file(output_file);
    }

    //
    // Create the actions.
    //
    Node actions;

    conduit::Node pipelines;
    // pipeline 1
    pipelines["pl1/f1/type"] = "contour";
    // filter knobs
    conduit::Node &contour_params = pipelines["pl1/f1/params"];
    contour_params["field"] = "braid";
    contour_params["iso_values"] = 0.;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"] = "pseudocolor";
    scenes["scene1/plots/plt1/pipeline"] = "pl1";
    scenes["scene1/plots/plt1/field"] = "braid";
    // setup required cinema params
    scenes["scene1/renders/r1/type"] = "cinema";
    scenes["scene1/renders/r1/phi_range"].set({-180., 180.});
    scenes["scene1/renders/r1/dphi"] = 90.;
    scenes["scene1/renders/r1/theta_range"].set({0., 180.});
    scenes["scene1/renders/r1/dtheta"] = 90.;
    scenes["scene1/renders/r1/db_name"] = "test_db2";
    //scenes["scene1/renders/r1/annotations"] = "false";
    scenes["scene1/renders/r1/camera/zoom"] = 1.0; // no zoom

    // add the pipeline
    conduit::Node &add_pipelines = actions.append();
    add_pipelines["action"] = "add_pipelines";
    add_pipelines["pipelines"] = pipelines;
    // add scene
    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    actions.print();

    //
    // Run Ascent
    //

    Ascent ascent;
    Node ascent_opts;
    // default is now ascent
    ascent_opts["runtime/type"] = "ascent";
    ascent.open(ascent_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();

    // check that we created an image
    EXPECT_TRUE(conduit::utils::is_file(output_file));
}

//-----------------------------------------------------------------------------
TEST(ascent_cinema_a, test_angle_range_2)
{
    // the vtkm runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent support disabled, skipping test");
        return;
    }

    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    std::string db_name = "test_db3";
    string output_path = "./cinema_databases/" + db_name;
    string output_file = conduit::utils::join_file_path(output_path, "info.json");
    // remove old file before rendering
    if(conduit::utils::is_file(output_file))
    {
        conduit::utils::remove_file(output_file);
    }

    //
    // Create the actions.
    //
    Node actions;

    conduit::Node pipelines;
    // pipeline 1
    pipelines["pl1/f1/type"] = "contour";
    // filter knobs
    conduit::Node &contour_params = pipelines["pl1/f1/params"];
    contour_params["field"] = "braid";
    contour_params["iso_values"] = 0.;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"] = "pseudocolor";
    scenes["scene1/plots/plt1/pipeline"] = "pl1";
    scenes["scene1/plots/plt1/field"] = "braid";
    // setup required cinema params
    scenes["scene1/renders/r1/type"] = "cinema";
    scenes["scene1/renders/r1/phi_range"].set({-180., 180.});
    scenes["scene1/renders/r1/phi_num_angles"] = 5;
    scenes["scene1/renders/r1/theta_range"].set({0., 180.});
    scenes["scene1/renders/r1/theta_num_angles"] = 3;
    scenes["scene1/renders/r1/db_name"] = "test_db3";
    //scenes["scene1/renders/r1/annotations"] = "false";
    scenes["scene1/renders/r1/camera/zoom"] = 1.0; // no zoom

    // add the pipeline
    conduit::Node &add_pipelines = actions.append();
    add_pipelines["action"] = "add_pipelines";
    add_pipelines["pipelines"] = pipelines;
    // add scene
    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    actions.print();

    //
    // Run Ascent
    //

    Ascent ascent;
    Node ascent_opts;
    // default is now ascent
    ascent_opts["runtime/type"] = "ascent";
    ascent.open(ascent_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();

    // check that we created an image
    EXPECT_TRUE(conduit::utils::is_file(output_file));
}

//-----------------------------------------------------------------------------
TEST(ascent_cinema_a, test_angle_values)
{
    // the vtkm runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent support disabled, skipping test");
        return;
    }

    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    std::string db_name = "test_db4";
    string output_path = "./cinema_databases/" + db_name;
    string output_file = conduit::utils::join_file_path(output_path, "info.json");
    // remove old file before rendering
    if(conduit::utils::is_file(output_file))
    {
        conduit::utils::remove_file(output_file);
    }

    //
    // Create the actions.
    //
    Node actions;

    conduit::Node pipelines;
    // pipeline 1
    pipelines["pl1/f1/type"] = "contour";
    // filter knobs
    conduit::Node &contour_params = pipelines["pl1/f1/params"];
    contour_params["field"] = "braid";
    contour_params["iso_values"] = 0.;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"] = "pseudocolor";
    scenes["scene1/plots/plt1/pipeline"] = "pl1";
    scenes["scene1/plots/plt1/field"] = "braid";
    // setup required cinema params
    scenes["scene1/renders/r1/type"] = "cinema";
    scenes["scene1/renders/r1/phi_angles"].set({-180., -90., 0., 90., 180.});
    scenes["scene1/renders/r1/theta_angles"].set({0., 90., 180.});
    scenes["scene1/renders/r1/db_name"] = "test_db4";
    //scenes["scene1/renders/r1/annotations"] = "false";
    scenes["scene1/renders/r1/camera/zoom"] = 1.0; // no zoom

    // add the pipeline
    conduit::Node &add_pipelines = actions.append();
    add_pipelines["action"] = "add_pipelines";
    add_pipelines["pipelines"] = pipelines;
    // add scene
    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    actions.print();

    //
    // Run Ascent
    //

    Ascent ascent;
    Node ascent_opts;
    // default is now ascent
    ascent_opts["runtime/type"] = "ascent";
    ascent.open(ascent_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();

    // check that we created an image
    EXPECT_TRUE(conduit::utils::is_file(output_file));
}

//-----------------------------------------------------------------------------
TEST(ascent_cinema_a, test_phi_theta_positions)
{
    // the vtkm runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent support disabled, skipping test");
        return;
    }

    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    std::string db_name = "test_db5";
    string output_path = "./cinema_databases/" + db_name;
    string output_file = conduit::utils::join_file_path(output_path, "info.json");
    // remove old file before rendering
    if(conduit::utils::is_file(output_file))
    {
        conduit::utils::remove_file(output_file);
    }

    //
    // Create the actions.
    //
    Node actions;

    conduit::Node pipelines;
    // pipeline 1
    pipelines["pl1/f1/type"] = "contour";
    // filter knobs
    conduit::Node &contour_params = pipelines["pl1/f1/params"];
    contour_params["field"] = "braid";
    contour_params["iso_values"] = 0.;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"] = "pseudocolor";
    scenes["scene1/plots/plt1/pipeline"] = "pl1";
    scenes["scene1/plots/plt1/field"] = "braid";
    // setup required cinema params
    scenes["scene1/renders/r1/type"] = "cinema";
    conduit::Node positions;
    positions.append().set({   0.,   0.});
    positions.append().set({-180.,  90.});
    positions.append().set({ -90.,  90.});
    positions.append().set({   0.,  90.});
    positions.append().set({  90.,  90.});
    positions.append().set({ 180.,  90.});
    positions.append().set({   0., 180.});
    scenes["scene1/renders/r1/phi_theta_positions"] = positions;
    scenes["scene1/renders/r1/db_name"] = "test_db5";
    //scenes["scene1/renders/r1/annotations"] = "false";
    scenes["scene1/renders/r1/camera/zoom"] = 1.0; // no zoom

    // add the pipeline
    conduit::Node &add_pipelines = actions.append();
    add_pipelines["action"] = "add_pipelines";
    add_pipelines["pipelines"] = pipelines;
    // add scene
    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    actions.print();

    //
    // Run Ascent
    //

    Ascent ascent;
    Node ascent_opts;
    // default is now ascent
    ascent_opts["runtime/type"] = "ascent";
    ascent.open(ascent_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();

    // check that we created an image
    EXPECT_TRUE(conduit::utils::is_file(output_file));
}

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    int result = 0;

    ::testing::InitGoogleTest(&argc, argv);

    // allow override of the data size via the command line
    if(argc == 2)
    {
        EXAMPLE_MESH_SIDE_DIM = atoi(argv[1]);
    }

    result = RUN_ALL_TESTS();
    return result;
}


