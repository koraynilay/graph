//=======================================================================
// Copyright (C) 2026 koraynilay
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// Regression test for sloan_ordering, including a minimal reproducer for
// the off-by-one bug in sloan_start_end_vertices where shrink_trace was
// allocated with size maximum_degree instead of maximum_degree + 1,
// causing an out-of-bounds access on regular graphs (all vertices share
// the same degree).

#include <boost/config.hpp>
#include <vector>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <boost/core/lightweight_test.hpp>

int main(int, char*[])
{
    using namespace boost;
    using namespace std;

    typedef adjacency_list< setS, vecS, undirectedS,
        property< vertex_color_t, default_color_type,
            property< vertex_degree_t, int,
                property< vertex_priority_t, double > > > >
        Graph;
    typedef graph_traits< Graph >::vertex_descriptor Vertex;
    typedef graph_traits< Graph >::vertices_size_type size_type;

    typedef property_map< Graph, vertex_index_t >::type IndexMap;

    // Helper: fill vertex_degree property from actual graph degrees.
    auto fill_degrees = [](Graph& g) {
        property_map< Graph, vertex_degree_t >::type deg
            = get(vertex_degree, g);
        graph_traits< Graph >::vertex_iterator ui, ui_end;
        for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
            deg[*ui] = degree(*ui, g);
    };

    // Helper: check that the ordering is a valid permutation of all vertices.
    auto assert_valid_permutation
        = [](const Graph& g, const std::vector< Vertex >& order) {
              IndexMap index_map = get(vertex_index, g);
              const size_type n = num_vertices(g);
              BOOST_TEST(order.size() == n);
              std::vector< bool > seen(n, false);
              for (std::size_t k = 0; k < order.size(); ++k)
              {
                  size_type idx = index_map[order[k]];
                  BOOST_TEST(idx < n);
                  if (idx < n)
                  {
                      BOOST_TEST(!seen[idx]);
                      seen[idx] = true;
                  }
              }
              for (size_type k = 0; k < n; ++k)
                  BOOST_TEST(seen[k]);
          };

    // ---- Test 1: 4-node cycle (minimal regular-graph reproducer) --------
    // All vertices have degree 2, so maximum_degree == 2.
    // The farthest vertex from the BFS root also has degree 2.
    // Before the fix, shrink_trace had size 2 (indices 0..1) but was
    // accessed at index 2 — an off-by-one out-of-bounds access.
    {
        const int n = 4;
        Graph G(n);
        for (int i = 0; i < n; ++i)
            add_edge(i, (i + 1) % n, G);
        fill_degrees(G);

        std::vector< Vertex > order(n);
        sloan_ordering(G, order.begin(), get(vertex_color, G),
            make_degree_map(G), get(vertex_priority, G));

        cout << "Sloan ordering (4-cycle):" << endl << "  ";
        IndexMap index_map = get(vertex_index, G);
        for (std::vector< Vertex >::const_iterator i = order.begin();
             i != order.end(); ++i)
            cout << index_map[*i] << " ";
        cout << endl;

        assert_valid_permutation(G, order);
    }

    // ---- Test 2: 6-node cycle (ring topology) ---------------------------
    {
        const int n = 6;
        Graph G(n);
        for (int i = 0; i < n; ++i)
            add_edge(i, (i + 1) % n, G);
        fill_degrees(G);

        std::vector< Vertex > order(n);
        sloan_ordering(G, order.begin(), get(vertex_color, G),
            make_degree_map(G), get(vertex_priority, G));

        cout << "Sloan ordering (6-cycle):" << endl << "  ";
        IndexMap index_map = get(vertex_index, G);
        for (std::vector< Vertex >::const_iterator i = order.begin();
             i != order.end(); ++i)
            cout << index_map[*i] << " ";
        cout << endl;

        assert_valid_permutation(G, order);
    }

    // ---- Test 3: non-regular graph (mixed degrees) ----------------------
    // A small graph where vertices have varying degrees, to verify sloan
    // ordering still works correctly on non-regular topologies.
    {
        typedef std::pair< std::size_t, std::size_t > Pair;
        Pair edges[] = { Pair(0, 1), Pair(0, 2), Pair(1, 2), Pair(1, 3),
            Pair(2, 4), Pair(3, 4), Pair(3, 5), Pair(4, 5) };

        const int n = 6;
        Graph G(n);
        for (int i = 0; i < 8; ++i)
            add_edge(edges[i].first, edges[i].second, G);
        fill_degrees(G);

        std::vector< Vertex > order(n);
        sloan_ordering(G, order.begin(), get(vertex_color, G),
            make_degree_map(G), get(vertex_priority, G));

        cout << "Sloan ordering (mixed-degree graph):" << endl << "  ";
        IndexMap index_map = get(vertex_index, G);
        for (std::vector< Vertex >::const_iterator i = order.begin();
             i != order.end(); ++i)
            cout << index_map[*i] << " ";
        cout << endl;

        assert_valid_permutation(G, order);
    }

    return boost::report_errors();
}
