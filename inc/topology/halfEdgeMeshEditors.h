#pragma once
#include "halfEdgeMeshData.h"
#include <list>

namespace gbs
{
    template <typename T, size_t dim>
    auto add_face(
        const std::shared_ptr<HalfEdgeFace<T, dim>> &face, 
        const std::shared_ptr<HalfEdge<T, dim>> &edge, 
        const std::array<T, dim> &coords )  -> std::shared_ptr< HalfEdgeFace<T, dim> >
    {
        if(!edge || edge->opposite || edge->face != face)
        {
            return nullptr;
        }
        
        auto opposite = make_opposite( edge->previous->vertex, edge);

        auto lst = { make_shared_h_edge( edge->vertex ), opposite, make_shared_h_edge( coords )};
        
        return make_shared_h_face<T,dim>(lst);
    }

    template <typename T, size_t dim>
    auto add_face(
        const std::shared_ptr<HalfEdge<T, dim>> &edge, 
        const std::array<T, dim> &coords )  -> std::shared_ptr< HalfEdgeFace<T, dim> >
    {
        if(!edge || edge->opposite)
        {
            return nullptr;
        }
        
        auto opposite = make_opposite( edge->previous->vertex, edge);

        auto lst = { make_shared_h_edge( edge->vertex ), opposite, make_shared_h_edge( coords )};
        
        return make_shared_h_face<T,dim>(lst);
    }

    template <typename T, size_t dim>
    void associate(std::shared_ptr<HalfEdgeVertex<T, dim>> &h_v, std::shared_ptr<HalfEdge<T, dim>> &h_e)
    {
        h_e->vertex = h_v;
        h_v->edge = h_e;
    }

    template <typename T, size_t dim>
    void flip(std::shared_ptr<HalfEdgeFace<T, dim>> &h_f1, std::shared_ptr<HalfEdgeFace<T, dim>> &h_f2)
    {
        assert(getFaceEdges(h_f1).size() == 3);
        assert(getFaceEdges(h_f2).size() == 3);
        auto [h_e1_1, h_e1_2] = getCommonEdges(h_f1, h_f2);
        if(h_e1_1 == nullptr || h_e1_2 == nullptr) return;
        auto h_e2_1 = h_e1_1->next;
        auto h_e3_1 = h_e2_1->next;
        auto h_e2_2 = h_e1_2->next;
        auto h_e3_2 = h_e2_2->next;

        auto h_v1 = h_e1_1->vertex;
        auto h_v2 = h_e2_1->vertex;
        auto h_v3 = h_e3_1->vertex;
        auto h_v4 = h_e2_2->vertex;

        associate(h_v4, h_e1_1);
        associate(h_v2, h_e1_2);

        std::list< std::shared_ptr<HalfEdge<T, dim>> > lst1{h_e1_1, h_e3_2, h_e2_1};
        std::list< std::shared_ptr<HalfEdge<T, dim>> > lst2{h_e1_2, h_e3_1, h_e2_2};

        make_loop(lst1.begin(), lst1.end(), h_f1);
        make_loop(lst2.begin(), lst2.end(), h_f2);

    }
    /**
     * @brief Link 2 half edges to as one edge
     * 
     * @tparam T 
     * @tparam dim 
     * @param h_e1 
     * @param h_e2 
     */
    template <typename T, size_t dim>
    void link_edges(const std::shared_ptr<HalfEdge<T, dim>> &h_e1, const std::shared_ptr<HalfEdge<T, dim>> &h_e2)
    {
        assert(h_e1);
        assert(h_e2);
        h_e1->opposite = h_e2;
        h_e2->opposite = h_e1;
    }

    template <typename T, size_t dim>
    auto add_vertex(const std::list<std::shared_ptr<HalfEdge<T, dim>> > &h_e_lst, const std::shared_ptr<HalfEdgeVertex<T, dim>> &h_v)
    {
        std::list<std::shared_ptr<HalfEdgeFace<T, dim>>> h_f_lst;

        std::shared_ptr<HalfEdge<T, dim>> h_e_prev{};
        h_v->edge = nullptr; // first new half edge takes ownership
        for(const auto &h_e : h_e_lst)
        {
            assert(h_e->previous);
            auto h_e1 = make_shared_h_edge(h_v);
            auto h_e2 = make_shared_h_edge(h_e->previous->vertex);
            if(h_e_prev)
            {
                link_edges(h_e2, h_e_prev);
            }
            h_e_prev = h_e1;
            auto lst = {h_e,h_e1, h_e2};
            h_f_lst.push_back(
                make_shared_h_face<T,dim>( lst )
            );
            assert(is_ccw(h_f_lst.back()));
        }
        link_edges(h_f_lst.back()->edge->next, h_f_lst.front()->edge->previous);
        return h_f_lst;
    }
}