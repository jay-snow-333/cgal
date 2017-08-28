#include <iostream>
#include <fstream>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/VSA_approximation.h>
#include <CGAL/Timer.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef boost::property_map<Polyhedron, boost::vertex_point_t>::type VertexPointMap;

typedef CGAL::VSA_approximation<Polyhedron, VertexPointMap> L21VSA;
typedef L21VSA::ErrorMetric L21Metric;
typedef L21VSA::ProxyFitting L21ProxyFitting;

typedef CGAL::Timer Timer;

/**
 * This file is a timing benchmark of each phase.
 * With different configuration:
   1. initialization
   2. number of proxies
   3. number of iterations
 * TODO: error
 */
int main(int argc, char *argv[])
{
  if (argc < 5)
    return 1;

  Polyhedron mesh;
  std::ifstream input(argv[1]);
  if (!input || !(input >> mesh) || mesh.empty()) {
    std::cerr << "Invalid off file." << std::endl;
    return 1;
  }
  std::cerr << "#triangles " << mesh.size_of_facets() << std::endl;

  // algorithm instance
  L21VSA l21_vsa(mesh,
    get(boost::vertex_point, const_cast<Polyhedron &>(mesh)));

  // set metric error and fitting functors
  L21Metric l21_metric(mesh);
  L21ProxyFitting l21_fitting(mesh);
  l21_vsa.set_metric(l21_metric, l21_fitting);

  int init = std::atoi(argv[2]);
  if (init < 0 || init > 2)
    return 1;
  const std::size_t num_proxies = std::atoi(argv[3]);
  const std::size_t num_iterations = std::atoi(argv[4]);
  std::cerr << "#init " << init << std::endl;
  std::cerr << "#num_proxies " << num_proxies << std::endl;
  std::cerr << "#num_iterations " << num_iterations << std::endl;

  Timer t0, t1;
  t1.start();

  std::cerr << "start initialization" << std::endl;
  t0.reset();
  t0.start();
  switch(init) {
    case 0:
      l21_vsa.seed_random(num_proxies);
      break;
    case 1:
      l21_vsa.seed_incremental(num_proxies, 5);
      break;
    case 2:
      l21_vsa.seed_hierarchical(num_proxies, 5);
      break;
    default:
      return 1;
  }
  t0.stop();
  std::cerr << "initialization time " << t0.time() << " sec." << std::endl;

  std::cerr << "start relaxation" << std::endl;
  t0.reset();
  t0.start();
  for (std::size_t i = 0; i < num_iterations; ++i)
    l21_vsa.run_one_step();
  t0.stop();
  std::cerr << "relaxation time " << t0.time() << " sec." << std::endl;

  Polyhedron mesh_out;
  std::cerr << "start relaxation" << std::endl;
  t0.reset();
  t0.start();
  l21_vsa.meshing(mesh_out);
  t0.stop();
  std::cerr << "meshing time " << t0.time() << " sec." << std::endl;

  std::cerr << "total time " << t1.time() << " sec." << std::endl;

  return 0;
}
