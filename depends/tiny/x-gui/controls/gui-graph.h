#pragma once

/****************************************************** //
//              tiny-for-c++ v3 library                 //
//              -----------------------                 //
//   Copyright (c) 2016-2024 | NEXTWave Technologies    //
//      <http://www.nextwave-techs.com/>                //
// ******************************************************/

//! Check if your project qualifies for a free license
//!   at http://nextwave-techs.com/license

//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//!        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//! SOFTWARE.

#ifndef TINY_GUI_GRAPH_H
#define TINY_GUI_GRAPH_H

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! GUI Graph View

//////////////////////////////////////////////////////////////////////////////
template<class TNode,class TEdge>
    //! @note must be defined => TNode::id_t ,id_t TNode::id() ,TNode &TEdge::a() ,TNode &TEdge::b()
struct GuiGraph_ : GuiControl {

///-- Nodes
    struct Node { //TODO could derive from instead of 'p'
        PtrOf<TNode> p;

        String label; //! drawn in the middle of the node (default is toString( p->id() ... )
        OsPoint pos;
    };

    typedef typename TNode::id_t nodeid_t;

    typedef MapOf<nodeid_t,Node> Nodes;

    Nodes nodes;

///-- Edges
    struct Edge {
        PtrOf<TEdge> p;

        String label; //! drawn on the edge center
        OsPoint p1 ,p2;
    };

    typedef typename TEdge::id_t edgeid_t;

    typedef MapOf<edgeid_t,Edge> Edges;

    Edges edges;

///-- Events
    struct IEvents {
        virtual bool onNodeClick( Node *node ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
        virtual bool onEdgeClick( Edge *edge ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    };

    IEvents *listener = NullPtr;

///--
    GuiGraph_( IEvents *alistener = NullPtr ) : listener( alistener ) {
        LoadBinary( "graph-pos.dat" ,positions );
    }

    ~GuiGraph_() {
        updatePosistions();

        SaveBinary( "graph-pos.dat" ,positions );
    }

    int nodeRadius = 32;

    OsColorRef colorEdge = OS_COLOR_LIGHTGRAY;
    OsColorRef colorNode = OS_COLOR_DARKGRAY;
    OsColorRef colorText = OS_COLOR_LIGHTGRAY;

    //--
    MapOf<nodeid_t,OsPoint> positions;

    void updatePosistions() {
        for( auto &it : nodes ) {
            auto &node = it.second;

            positions[ it.first ] = node.pos;
        }
    }

///--
    void addNode( PtrOf<TNode> &node ) {
        const auto &it_pos = positions.find( node->id() );

        auto x = Node({node});

        if( it_pos != positions.end() ) {
            x.pos.x = it_pos->second.x;
            x.pos.y = it_pos->second.y;
        } else {
            int w = area().right - area().left;
            int h = area().bottom - area().top;

            x.pos.x = rand() % MAX(w,2*nodeRadius);
            x.pos.y = rand() % MAX(h,2*nodeRadius);

            positions[ node->id() ] = x.pos;
        }

        x.label = node->id();

        nodes[ node->id() ] = x;
    }

    Node *findNode( const nodeid_t &id ) {
        auto &it = nodes.find( id );

        return (it != nodes.end()) ? &(it->second) : NullPtr;
    }

    //--
    void addEdge( PtrOf<TEdge> &edge ) {
        auto x = Edge({edge});

        x.p1 = nodes[ edge->a()->id() ].pos;
        x.p2 = nodes[ edge->b()->id() ].pos;

        edges[ edge->id() ] = x;
    }

    Edge *findEdge( const edgeid_t &id ) {
        auto &it = edges.find( id );

        return (it != edges.end()) ? &(it->second) : nullptr;
    }

    //--
    void clearAll() {
        nodes.clear();
        edges.clear();
    }

///--
    OsPoint edgeSelectDim = { 64 ,32 };

    Node *m_selectNode = NullPtr; //! later ... multi select
    Edge *m_selectEdge = NullPtr;
    Node *m_dragNode = NullPtr;

    void onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {
        GuiControl::onClick( p ,mouseButton ,keyState );

        const int r = nodeRadius;

        const bool selecting = ( mouseButton == OsMouseButton::osLeftMouseButton && keyStateValue(keyState) == 0 );
        const bool dragging = ( mouseButton == OsMouseButton::osLeftMouseButton && keyState.alt );

        ///-- nodes
        for( auto &it : nodes ) {
            Node &node = it.second;

            OsPoint center = { area().left+node.pos.x ,area().top+node.pos.y };
            OsRect rect = { center.x-r ,center.y-r ,center.x+r ,center.y+r };

            if( !TestHit( p ,rect ) ) continue;

            if( dragging ) {
                m_dragNode = &node; return;
            } else {
                bool haveClick =
                    listener ? listener->onNodeClick( &node ,mouseButton ,keyState ) : true
                ;

                if( !haveClick ) continue;

                if( selecting )
                    m_selectNode = &node;

                return;
            }
        }

        if( selecting && m_selectNode ) { //! unselecting node
            m_selectNode = NullPtr;

            SAFECALL(listener)->onNodeClick( m_selectNode ,mouseButton ,keyState );
        }

        ///-- edges
        const int w2 = edgeSelectDim.x / 2;
        const int h2 = edgeSelectDim.y / 2;

        for( auto &it : edges ) {
            Edge &edge = it.second;

            OsPoint p1 = { area().left+edge.p1.x ,area().top+edge.p1.y };
            OsPoint p2 = { area().left+edge.p2.x ,area().top+edge.p2.y };

            OsPoint center = {
                p1.x + (p2.x - p1.x) / 2
                ,p1.y + (p2.y - p1.y) / 2
            };

            OsRect rect = { center.x-w2 ,center.y-h2 ,center.x+w2 ,center.y+h2 };

            if( !TestHit( p ,rect ) ) continue;

            bool haveClick =
                listener ? listener->onEdgeClick( &edge ,mouseButton ,keyState ) : true
            ;

            if( !haveClick ) continue;

            if( selecting )
                m_selectEdge = &edge;

            return;
        }

        if( selecting && m_selectEdge ) { //! unselecting edge
            m_selectEdge = NullPtr;

            SAFECALL(listener)->onEdgeClick( m_selectEdge ,mouseButton ,keyState );
        }
    }

    void onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {
        GuiControl::onMouseMove( p ,mouseButton ,keyState );

        //TODO show hoover

        if( m_dragNode ) {
            m_dragNode->pos = { p.x - area().left ,p.y - area().top };

            for( auto &it : edges ) {
                auto &edge = it.second;

                if( edge.p->a()->id() == m_dragNode->p->id() ) {
                    edge.p1 = m_dragNode->pos;
                }
                if( edge.p->b()->id() == m_dragNode->p->id() ) {
                    edge.p2 = m_dragNode->pos;
                }
            }

            Refresh();
        }
    }

    void onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {
        GuiControl::onMouseUp( p ,mouseButton ,keyState );

        if( m_dragNode ) {
            m_dragNode = nullptr;
        }

        Refresh();
    }

///--
    void DrawArrowTip( const OsPoint &p1 ,const OsPoint &p2 ) {
        double dx = (double) (p2.x - p1.x) ,dy = (double) (p2.y - p1.y);

        double r = sqrt( dx*dx + dy*dy );

        int px = p2.x - 36 * dx/r;
        int py = p2.y - 36 * dy/r;

        //--
        int tx = px + 16 * dy/r;
        int ty = py - 16 * dx/r;

        root().DrawLine( px ,py ,tx ,ty );

        //--
        tx = px - 16 * dy/r;
        ty = py + 16 * dx/r;

        root().DrawLine( px ,py ,tx ,ty );
    }

    //--
    virtual void onDrawNode( Node &node ,const OsPoint &p ) {
        const int r = nodeRadius;

        root().DrawEllipse( p.x-r ,p.y-r ,p.x+r ,p.y+r );

        if( !node.label.empty() )
            root().DrawTextAlign( node.label.c_str() ,p.x-r ,p.y-r ,p.x+r ,p.y+r ,(TextAlign) (textalignCenterH | textalignCenterV) );
    }

    virtual void onDrawEdge( Edge &edge ,const OsPoint &p1 ,const OsPoint &p2 ) {
        root().DrawLine( p1.x ,p1.y ,p2.x ,p2.y );

        if( edge.p->directed() )
            DrawArrowTip( p1 ,p2 );

        if( !edge.label.empty() )
            root().DrawTextAlign( edge.label.c_str() ,p1.x ,p1.y ,p2.x ,p2.y ,(TextAlign) (textalignCenterH | textalignCenterV) );
    }

    void onDraw( const OsRect &uptadeArea ) override {
        GuiControl::onDraw( uptadeArea );

        root().SetTextColor( colorText );

        ///-- edges
        for( auto &it : edges ) {
            auto &edge = it.second;

            root().SetForeColor( colorEdge );
            root().SetFillColor( colorEdge );

            OsPoint p1 = { area().left+edge.p1.x ,area().top+edge.p1.y };
            OsPoint p2 = { area().left+edge.p2.x ,area().top+edge.p2.y };

            onDrawEdge( edge ,p1 ,p2 );
        }

        ///-- nodes
        for( auto &it : nodes ) {
            auto &node = it.second;

            root().SetForeColor( colorNode );
            root().SetFillColor( colorNode );

            OsPoint p = { area().left+node.pos.x ,area().top+node.pos.y };

            onDrawNode( node ,p );
        }
    }
};

//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_GRAPH_H