//=======================================================================
// Copyright (C) 2026 koraynilay
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/config.hpp>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <boost/core/lightweight_test.hpp>

int main(int, char*[])
{
    using namespace boost;

    typedef adjacency_list< setS, vecS, undirectedS,
        property< vertex_color_t, default_color_type,
            property< vertex_priority_t, double > > >
        Graph;
    typedef graph_traits< Graph >::vertex_descriptor Vertex;

    // Test 1: 4-node cycle
    // Regression test for off-by-one bug in sloan_start_end_vertices where
    // shrink_trace was allocated with size maximum_degree instead of
    // maximum_degree + 1, causing an out-of-bounds access when a peripheral
    // vertex at maximum distance has degree == maximum_degree.
    {
        const int n = 4;
        Graph G(n);
        add_edge(0, 1, G);
        add_edge(1, 2, G);
        add_edge(2, 3, G);
        add_edge(3, 0, G);

        std::vector< Vertex > order(n);
        sloan_ordering(G, order.begin(), get(vertex_color, G),
            make_degree_map(G), get(vertex_priority, G));

        const std::vector< Vertex > expected = { 0, 3, 1, 2 };
        BOOST_TEST_ALL_EQ(
            order.begin(), order.end(), expected.begin(), expected.end());
    }

    // Test 2: 10-node irregular graph (same graph used in king and
    // cuthill-mckee ordering tests)
    {
        typedef std::pair< std::size_t, std::size_t > Pair;
        constexpr Pair edges[14] = { Pair(0, 3), // a-d
            Pair(0, 5), // a-f
            Pair(1, 2), // b-c
            Pair(1, 4), // b-e
            Pair(1, 6), // b-g
            Pair(1, 9), // b-j
            Pair(2, 3), // c-d
            Pair(2, 4), // c-e
            Pair(3, 5), // d-f
            Pair(3, 8), // d-i
            Pair(4, 6), // e-g
            Pair(5, 6), // f-g
            Pair(5, 7), // f-h
            Pair(6, 7) }; // g-h

        const int n = 10;
        Graph G(n);
        for (int i = 0; i < 14; ++i)
            add_edge(edges[i].first, edges[i].second, G);

        std::vector< Vertex > order(n);
        sloan_ordering(G, order.begin(), get(vertex_color, G),
            make_degree_map(G), get(vertex_priority, G));

        const std::vector< Vertex > expected = { 8, 0, 3, 7, 5, 2, 4, 6, 1, 9 };
        BOOST_TEST_ALL_EQ(
            order.begin(), order.end(), expected.begin(), expected.end());
    }

    return boost::report_errors();
}
