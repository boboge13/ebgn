/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

/*rbtree changed from linux kernel rbtree.c*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cmisc.h"

#include "crb.h"

/*for safe reason, when data handler is not given, set to default null function*/
static void __crb_data_null_default(void *data)
{
    return;
}

/*for safe reason, when data handler is not given, set to default print function*/
static void __crb_data_print_default(LOG *log, const void *data)
{
    sys_log(log, "data = %u\n", data);
    return;
}

static int __crb_data_cmp_default(const void *data_1, const void *data_2)
{
    if(data_1 > data_2)
    {
        return (1);
    }

    if(data_1 < data_2)
    {
        return (-1);
    }
    
    return (0);
}

CRB_NODE * crb_node_new()
{    
    CRB_NODE *node;

    alloc_static_mem(MD_TBD, ERR_MODULE_ID, MM_CRB_NODE, &node, LOC_CRB_0001);
    if(NULL_PTR == node)
    {
        sys_log(LOGSTDOUT, "error:crb_node_new: new node failed\n");
        return (NULL_PTR);
    }   
    crb_node_init(node);
    return (node);
}

void crb_node_init(CRB_NODE *node)
{    
    CRB_NODE_PARENT(node) = NULL_PTR;
    CRB_NODE_RIGHT(node)  = NULL_PTR;
    CRB_NODE_LEFT(node)   = NULL_PTR;
    CRB_NODE_COLOR(node)  = CRB_BLACK;
    CRB_NODE_DATA(node)   = NULL_PTR;
    return;
}

void crb_node_clean(CRB_NODE *node)
{
    CRB_NODE_PARENT(node) = NULL_PTR;
    CRB_NODE_RIGHT(node)  = NULL_PTR;
    CRB_NODE_LEFT(node)   = NULL_PTR;
    CRB_NODE_COLOR(node)  = CRB_BLACK;
    CRB_NODE_DATA(node)   = NULL_PTR;

    return;
}

void crb_node_free(CRB_NODE *node)
{
    if(NULL_PTR != node)
    {  
        crb_node_clean(node);
        free_static_mem(MD_TBD, ERR_MODULE_ID, MM_CRB_NODE, node, LOC_CRB_0002);        
    }

    return;
}

void crb_node_print(LOG *log, const CRB_TREE *crbtree, const CRB_NODE *node)
{
    sys_log(log, "node %p: parent = %p, left = %p, right = %p, color = %s, data = %p\n",
                       node,
                       CRB_NODE_PARENT(node),
                       CRB_NODE_LEFT(node),
                       CRB_NODE_RIGHT(node),
                       (CRB_NODE_IS_RED(node) ? "red  " : "black"),
                       CRB_NODE_DATA(node)
                       );
    CRB_TREE_DATA_PRINT(crbtree)(log, CRB_NODE_DATA(node));
    return;
}

void crb_node_print_level(LOG *log, const CRB_TREE *crbtree, const CRB_NODE *node, const uint16_t level)
{
    c_ident_print(log, level);
    sys_print(log, "%p: parent = %p, left = %p, right = %p, color = %s, data = %p\n",
                       node,
                       CRB_NODE_PARENT(node),
                       CRB_NODE_LEFT(node),
                       CRB_NODE_RIGHT(node),
                       CRB_NODE_IS_RED(node)  ? "red  " : "black",
                       CRB_NODE_DATA(node)
                       );
    c_ident_print(log, level);                       
    CRB_TREE_DATA_PRINT(crbtree)(log, CRB_NODE_DATA(node));                       
    return;                       
}


static void __crb_tree_rotate_left(CRB_TREE *crbtree, CRB_NODE *node)
{
    CRB_NODE *right;

    right = CRB_NODE_RIGHT(node);

    if(NULL_PTR != (CRB_NODE_RIGHT(node) = CRB_NODE_LEFT(right)))
    {
        CRB_NODE *left;
        left = CRB_NODE_LEFT(right);
        CRB_NODE_PARENT(left) = node;
    }
    CRB_NODE_LEFT(right) = node;

    if(NULL_PTR != (CRB_NODE_PARENT(right) = CRB_NODE_PARENT(node)))
    {
        CRB_NODE *parent;
        parent = CRB_NODE_PARENT(node);
        
        if (node == CRB_NODE_LEFT(parent))
        {
            CRB_NODE_LEFT(parent) = right;
        }
        else
        {
            CRB_NODE_RIGHT(parent) = right;
        }
    }
    else
    {
        CRB_TREE_ROOT(crbtree) = right;
    }
    CRB_NODE_PARENT(node) = right;
    return;
}

static void __crb_tree_rotate_right(CRB_TREE *crbtree, CRB_NODE *node)
{
    CRB_NODE *left;

    left = CRB_NODE_LEFT(node);

    if (NULL_PTR != (CRB_NODE_LEFT(node) = CRB_NODE_RIGHT(left)))
    {
        CRB_NODE *right;
        right = CRB_NODE_RIGHT(left);
        CRB_NODE_PARENT(right) = node;
    }
    CRB_NODE_RIGHT(left) = node;

    if (NULL_PTR != (CRB_NODE_PARENT(left) = CRB_NODE_PARENT(node)))
    {
        CRB_NODE *parent;
        parent = CRB_NODE_PARENT(node);
    
        if (node == CRB_NODE_RIGHT(parent))
        {
            CRB_NODE_RIGHT(parent) = left;
        }
        else
        {
            CRB_NODE_LEFT(parent) = left;
        }
    }
    else
    {
        CRB_TREE_ROOT(crbtree) = left;
    }
    CRB_NODE_PARENT(node) = left;
    return;
}

static void __crb_tree_insert_color(CRB_TREE *crbtree, CRB_NODE *node)
{
    CRB_NODE *parent;    
    
    while (NULL_PTR != (parent = CRB_NODE_PARENT(node)) /*parent is valid*/
         && CRB_RED == CRB_NODE_COLOR(parent))
    {
        CRB_NODE *gparent;

        parent = CRB_NODE_PARENT(node);

        gparent = CRB_NODE_PARENT(parent);
        ASSERT(NULL_PTR != gparent);

        if (parent == CRB_NODE_LEFT(gparent))
        {
            {
                CRB_NODE *uncle;
                if (NULL_PTR != (uncle = CRB_NODE_RIGHT(gparent)) /*uncle is valid*/
                   && CRB_RED == CRB_NODE_COLOR(uncle))
                {
                    CRB_NODE_COLOR(uncle)   = CRB_BLACK;
                    CRB_NODE_COLOR(parent)  = CRB_BLACK;
                    CRB_NODE_COLOR(gparent) = CRB_RED;
                    
                    node = gparent;
                    continue;
                }
            }

            if (CRB_NODE_RIGHT(parent) == node)
            {
                __crb_tree_rotate_left(crbtree, parent);
                XCHG(CRB_NODE *, parent, node);
            }

            CRB_NODE_COLOR(parent)  = CRB_BLACK;
            CRB_NODE_COLOR(gparent) = CRB_RED;
            __crb_tree_rotate_right(crbtree, gparent);
         }
         else 
         {        
            {
                CRB_NODE *uncle;
                if (NULL_PTR != (uncle = CRB_NODE_LEFT(gparent)) /*uncle is valid*/
                    && CRB_RED == CRB_NODE_COLOR(uncle))
                {
                    CRB_NODE_COLOR(uncle)   = CRB_BLACK;
                    CRB_NODE_COLOR(parent)  = CRB_BLACK;
                    CRB_NODE_COLOR(gparent) = CRB_RED;
                    
                    node = gparent;
                    continue;
                }
            }

            if (CRB_NODE_LEFT(parent) == node)
            {
                __crb_tree_rotate_right(crbtree, parent);
                XCHG(CRB_NODE *, parent, node);
            }

            CRB_NODE_COLOR(parent)  = CRB_BLACK;
            CRB_NODE_COLOR(gparent) = CRB_RED;
            __crb_tree_rotate_left(crbtree, gparent);
        }
    }

    CRB_NODE_COLOR(CRB_TREE_ROOT(crbtree)) = CRB_BLACK;
    return;
}

static void __crb_tree_erase_color(CRB_TREE *crbtree, CRB_NODE *node, CRB_NODE *parent)
{ 
    while ((NULL_PTR == node || CRB_BLACK == CRB_NODE_COLOR(node)) && node != CRB_TREE_ROOT(crbtree))
    {        
        if (CRB_NODE_LEFT(parent) == node)
        {
            CRB_NODE *other;
            CRB_NODE *o_left;
            CRB_NODE *o_right;
        
            other = CRB_NODE_RIGHT(parent);
            
            if (CRB_RED == CRB_NODE_COLOR(other))
            {
                CRB_NODE_COLOR(other)  = CRB_BLACK;
                CRB_NODE_COLOR(parent) = CRB_RED;
                
                __crb_tree_rotate_left(crbtree, parent);

                other = CRB_NODE_RIGHT(parent);
            }

            o_left = CRB_NODE_LEFT(other);
            o_right = CRB_NODE_RIGHT(other);

            if((NULL_PTR == o_left || CRB_BLACK == CRB_NODE_COLOR(o_left)) 
            && (NULL_PTR == o_right || CRB_BLACK == CRB_NODE_COLOR(o_right)))
            {
                CRB_NODE_COLOR(other) = CRB_RED;
                
                node = parent;                
                parent = CRB_NODE_PARENT(node);
            }
            else
            {
                if (NULL_PTR == o_right || CRB_BLACK == CRB_NODE_COLOR(o_right))
                {
                    if (NULL_PTR != o_left)
                    {
                        CRB_NODE_COLOR(o_left) = CRB_BLACK;
                    }
                    CRB_NODE_COLOR(other) = CRB_RED;
                    
                    __crb_tree_rotate_right(crbtree, other);
                    
                    other = CRB_NODE_RIGHT(parent);
                    /*note: other was changed here*/
                }
                
                CRB_NODE_COLOR(other) = CRB_NODE_COLOR(parent);
                CRB_NODE_COLOR(parent) = CRB_BLACK;

                /*due to other may be changed before, have to get o_right again here*/
                o_right = CRB_NODE_RIGHT(other);
                if (NULL_PTR != o_right)
                {
                    CRB_NODE_COLOR(o_right) = CRB_BLACK;
                }
                
                __crb_tree_rotate_left(crbtree, parent);
                node = CRB_TREE_ROOT(crbtree);
                break;
            }
        }
        else
        {
            CRB_NODE *other;
            CRB_NODE *o_left;
            CRB_NODE *o_right;
            
            other = CRB_NODE_LEFT(parent);
            
            if (CRB_RED == CRB_NODE_COLOR(other))
            {
                CRB_NODE_COLOR(other) = CRB_BLACK;
                CRB_NODE_COLOR(parent) = CRB_RED;
                
                __crb_tree_rotate_right(crbtree, parent);
                
                other = CRB_NODE_LEFT(parent);
            }

            o_left = CRB_NODE_LEFT(other);
            o_right = CRB_NODE_RIGHT(other);
            
            if ((NULL_PTR == o_left  || CRB_BLACK == CRB_NODE_COLOR(o_left))
             && (NULL_PTR == o_right || CRB_BLACK == CRB_NODE_COLOR(o_right)))
            {
                CRB_NODE_COLOR(other) = CRB_RED;
                
                node = parent;                
                parent = CRB_NODE_PARENT(node);
            }
            else
            {
                if (NULL_PTR == o_left  || CRB_BLACK == CRB_NODE_COLOR(o_left))
                {
                    if (NULL_PTR != o_right)
                    {
                        CRB_NODE_COLOR(o_right) = CRB_BLACK;
                    }
                    
                    CRB_NODE_COLOR(other) = CRB_RED;
                    
                    __crb_tree_rotate_left(crbtree, other);
                    
                    other = CRB_NODE_LEFT(parent);
                    /*note: other was changed here*/
                }
                
                CRB_NODE_COLOR(other) = CRB_NODE_COLOR(parent);
                CRB_NODE_COLOR(parent) = CRB_BLACK;

                /*due to other may be changed before, have to get o_left again here*/
                o_left = CRB_NODE_LEFT(other);
                if (NULL_PTR != o_left)
                {
                    CRB_NODE_COLOR(o_left) = CRB_BLACK;
                }
                __crb_tree_rotate_right(crbtree, parent);
                node = CRB_TREE_ROOT(crbtree);
                break;
            }
        }
    }

    if (NULL_PTR != node)
    {
        CRB_NODE_COLOR(node) = CRB_BLACK;
    }
    return;
}

static void __crb_tree_erase(CRB_TREE *crbtree, CRB_NODE *node)
{
    CRB_NODE * child;
    CRB_NODE * parent;
    uint16_t color;

    ASSERT(NULL_PTR != node);

    if (NULL_PTR == CRB_NODE_LEFT(node))
    {
        child = CRB_NODE_RIGHT(node);
    }
    else if (NULL_PTR == CRB_NODE_RIGHT(node))
    {
        child = CRB_NODE_LEFT(node);
    }
    else
    {
        CRB_NODE *old;
        CRB_NODE *left;

        old = node;        
        node = CRB_NODE_RIGHT(node);
        
        while (NULL_PTR != (left = CRB_NODE_LEFT(node)))
        {
            node = left;            
        }
        
        child  = CRB_NODE_RIGHT(node);
        parent = CRB_NODE_PARENT(node);
        color  = CRB_NODE_COLOR(node);

        if (NULL_PTR != child)
        {
            CRB_NODE_PARENT(child) = parent;
        }
        
        if (NULL_PTR != parent)
        {
            if (CRB_NODE_LEFT(parent) == node)
            {
                CRB_NODE_LEFT(parent) = child;
            }
            else
            {
                CRB_NODE_RIGHT(parent) = child;
            }
        }
        else
        {
            CRB_TREE_ROOT(crbtree) = child;
        }

        if (CRB_NODE_PARENT(node) == old)
        {
            parent = node;
        }
       
        CRB_NODE_PARENT(node) = CRB_NODE_PARENT(old);
        CRB_NODE_COLOR(node)  = CRB_NODE_COLOR(old);
        CRB_NODE_RIGHT(node)  = CRB_NODE_RIGHT(old);
        CRB_NODE_LEFT(node)   = CRB_NODE_LEFT(old);

        if (NULL_PTR != CRB_NODE_PARENT(old))
        {
            CRB_NODE *old_parent;
            old_parent = CRB_NODE_PARENT(old);
            
            if (CRB_NODE_LEFT(old_parent) == old)
            {
                CRB_NODE_LEFT(old_parent) = node;
            }
            else
            {
                CRB_NODE_RIGHT(old_parent) = node;
            }
        } 
        else
        {
            CRB_TREE_ROOT(crbtree) = node;
        }

        {
            CRB_NODE *old_left;

            old_left = CRB_NODE_LEFT(old);
            CRB_NODE_PARENT(old_left) = node;
        }        

        if (NULL_PTR != CRB_NODE_RIGHT(old))
        {
            CRB_NODE *old_right;
            old_right = CRB_NODE_RIGHT(old);
            CRB_NODE_PARENT(old_right) = node;
        }
        goto color;
    }

    parent = CRB_NODE_PARENT(node);
    color = CRB_NODE_COLOR(node);

    if (NULL_PTR != child)
    {
        CRB_NODE_PARENT(child) = parent;
    }
    
    if (NULL_PTR != parent)
    {
        if (CRB_NODE_LEFT(parent) == node)
        {
            CRB_NODE_LEFT(parent) = child;
        }
        else
        {
            CRB_NODE_RIGHT(parent) = child;
        }
    }
    else
    {
        CRB_TREE_ROOT(crbtree) = child;
    }

 color:
    if (CRB_BLACK == color)
    {
        __crb_tree_erase_color(crbtree, child, parent);
    }
    return;
}

EC_BOOL crb_tree_is_empty(const CRB_TREE *crbtree)
{
    if (0 == CRB_TREE_NODE_NUM(crbtree) || NULL_PTR == CRB_TREE_ROOT(crbtree))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

static uint16_t __crb_tree_node_count(const CRB_TREE *crbtree, const CRB_NODE *node)
{
    if(NULL_PTR == node)
    {
        return ((uint16_t)0);
    }

    return (uint16_t)(1 + __crb_tree_node_count(crbtree, CRB_NODE_LEFT(node)) + __crb_tree_node_count(crbtree, CRB_NODE_RIGHT(node)));   
}

uint16_t crb_tree_node_count(const CRB_TREE *crbtree)
{
    return __crb_tree_node_count(crbtree, CRB_TREE_ROOT(crbtree));
}

uint16_t crb_tree_node_num(const CRB_TREE *crbtree)
{
    return CRB_TREE_NODE_NUM(crbtree);
}

/*
 * This function returns the first node_pos (in sort order) of the tree.
 */
const CRB_NODE * crb_tree_first_node(const CRB_TREE *crbtree)
{
    const CRB_NODE *node;

    node = CRB_TREE_ROOT(crbtree);
    if (NULL_PTR == node)
    {
        return (NULL_PTR);
    }
    
    while (NULL_PTR != CRB_NODE_LEFT(node))
    {
        node = CRB_NODE_LEFT(node);
    }
    return (node);
}

const CRB_NODE * crb_tree_last_node(const CRB_TREE *crbtree)
{
    const CRB_NODE *node;

    node = CRB_TREE_ROOT(crbtree);
    if (NULL_PTR == node)
    {
        return (NULL_PTR);
    }
   
    while (NULL_PTR != CRB_NODE_RIGHT(node))
    {
        node = CRB_NODE_RIGHT(node);
    }
    
    return (node);
}

const CRB_NODE * crb_tree_next_node(const CRB_TREE *crbtree, const CRB_NODE *node)
{
    const CRB_NODE *node_t;
    const CRB_NODE *parent;

    node_t = node;

    /* If we have a right-hand child, go down and then left as far
       as we can. */
    if (NULL_PTR != CRB_NODE_RIGHT(node_t)) 
    {
        node_t = CRB_NODE_RIGHT(node_t);
        while (NULL_PTR != CRB_NODE_LEFT(node_t))
        {
            node_t = CRB_NODE_LEFT(node_t);
        }
        return (node_t);
    }

    /* No right-hand children.  Everything down and left is
       smaller than us, so any 'next' node_t must be in the general
       direction of our parent. Go up the tree; any time the
       ancestor is a right-hand child of its parent, keep going
       up. First time it's a left-hand child of its parent, said
       parent is our 'next' node_t. */
    while (NULL_PTR != (parent = CRB_NODE_PARENT(node_t)) && node_t == CRB_NODE_RIGHT(parent))
    {
        node_t = parent;
    }
    
    return (CRB_NODE_PARENT(node_t));
}

const CRB_NODE * crb_tree_prev_node(const CRB_TREE *crbtree, const CRB_NODE *node)
{
    const CRB_NODE *node_t;
    const CRB_NODE *parent;

    node_t = node;
    
    /* If we have a left-hand child, go down and then right as far
       as we can. */
    if (NULL_PTR != CRB_NODE_LEFT(node_t)) 
    {
        node_t = CRB_NODE_LEFT(node_t);
        while (NULL_PTR != CRB_NODE_RIGHT(node_t))
        {
            node_t = CRB_NODE_RIGHT(node_t);
        }
        return (node_t);
    }

    /* No left-hand children. Go up till we find an ancestor which
       is a right-hand child of its parent */
    while (NULL_PTR != (parent = CRB_NODE_PARENT(node_t)) && node_t == CRB_NODE_LEFT(parent))
    {
        node_t = parent;
    }

    return (CRB_NODE_PARENT(node_t));
}

void * crb_tree_first_data(const CRB_TREE *crbtree)
{
    const CRB_NODE *node;

    node = crb_tree_first_node(crbtree);
    if(NULL_PTR == node)
    {
        return (NULL_PTR);
    }
    return (void *)CRB_NODE_DATA(node);
}

void * crb_tree_last_data(const CRB_TREE *crbtree)
{
    const CRB_NODE *node;

    node = crb_tree_last_node(crbtree);
    if(NULL_PTR == node)
    {
        return (NULL_PTR);
    }
    return (void *)CRB_NODE_DATA(node);
}

void * crb_tree_next_data(const CRB_TREE *crbtree, const void *data)
{
    const CRB_NODE *node;

    node = crb_tree_search_data(crbtree, data);
    if(NULL_PTR == node)
    {
        return (NULL_PTR);
    }

    node = crb_tree_next_node(crbtree, node);
    if(NULL_PTR == node)
    {
        return (NULL_PTR);
    }    
    return (void *)CRB_NODE_DATA(node);
}

void * crb_tree_prev_data(const CRB_TREE *crbtree, const void *data)
{
    const CRB_NODE *node;

    node = crb_tree_search_data(crbtree, data);
    if(NULL_PTR == node)
    {
        return (NULL_PTR);
    }

    node = crb_tree_prev_node(crbtree, node);
    if(NULL_PTR == node)
    {
        return (NULL_PTR);
    }    
    return (void *)CRB_NODE_DATA(node);
}

/*victim_pos should be free*/
void crb_tree_replace_node(CRB_TREE *crbtree, CRB_NODE *victim, CRB_NODE *new_node)
{
    /* Set the surrounding nodes to point to the replacement */
    if (NULL_PTR != CRB_NODE_PARENT(victim)) 
    {
        CRB_NODE *parent;
        parent = CRB_NODE_PARENT(victim);
        
        if (victim == CRB_NODE_LEFT(parent))
        {
            CRB_NODE_LEFT(parent) = new_node;
        }
        else
        {
            CRB_NODE_RIGHT(parent) = new_node;
        }
    } 
    else 
    {
        CRB_TREE_ROOT(crbtree) = new_node;
    }
    
    if (NULL_PTR != CRB_NODE_LEFT(victim))
    {
        CRB_NODE *left;
        left = CRB_NODE_LEFT(victim);
        CRB_NODE_PARENT(left) = new_node;
    }
    if (NULL_PTR != CRB_NODE_RIGHT(victim))
    {
        CRB_NODE *right;
        right = CRB_NODE_RIGHT(victim);
        CRB_NODE_PARENT(right) = new_node;
    }

    return;
}

/*return the searched pos*/
CRB_NODE *crb_tree_search(CRB_TREE *crbtree, const CRB_NODE *node)
{
    CRB_NODE *node_t;
    void     *data;

    data   = CRB_NODE_DATA(node);
    node_t = CRB_TREE_ROOT(crbtree);
    
    while (NULL_PTR != node_t)
    {        
        int cmp;

        cmp = CRB_TREE_DATA_CMP(crbtree)(CRB_NODE_DATA(node_t), data);
        if (0 < cmp) /*data < CRB_NODE_DATA(node_t)*/
        {
            node_t = CRB_NODE_LEFT(node_t);
        }
        else if (0 > cmp)/*data > CRB_NODE_DATA(node_t)*/
        {
            node_t = CRB_NODE_RIGHT(node_t);
        }
        else
        {
            return (node_t);
        }
    }

    return (NULL_PTR);
}

CRB_NODE * crb_tree_insert(CRB_TREE *crbtree, CRB_NODE *new_node)
{
    CRB_NODE *node;
    CRB_NODE *parent;
    uint16_t  flag; /*0: on left subtree, 1: on right subtree*/

    node   = CRB_TREE_ROOT(crbtree);
    parent = NULL_PTR;

    while (NULL_PTR != node)
    {
        int cmp;
        
        cmp = CRB_TREE_DATA_CMP(crbtree)(CRB_NODE_DATA(node), CRB_NODE_DATA(new_node));

        parent = node;
        
        if (0 < cmp)/*data < CRB_NODE_DATA(node)*/
        {
            node = CRB_NODE_LEFT(node);
            flag = 0;
        }
        else if (0 > cmp)/*data > CRB_NODE_DATA(node)*/
        {
            node = CRB_NODE_RIGHT(node);
            flag = 1;
        }
        else
        {
            return (node);
        }
    }

    /*not found data in the rbtree*/
    CRB_NODE_PARENT(new_node) = parent;
    CRB_NODE_COLOR(new_node)      = CRB_RED;
    CRB_NODE_LEFT(new_node)   = NULL_PTR;
    CRB_NODE_RIGHT(new_node)  = NULL_PTR;        

    if(NULL_PTR == CRB_TREE_ROOT(crbtree))
    {
        CRB_TREE_ROOT(crbtree) = new_node;
    }
    else
    {
        if(0 == flag)/*on left subtree*/
        {
            CRB_NODE_LEFT(parent) = new_node;
        }
        else
        {
            CRB_NODE_RIGHT(parent) = new_node;
        }
    }
    __crb_tree_insert_color(crbtree, new_node);

    CRB_TREE_NODE_NUM(crbtree) ++;

    return (new_node);
}

EC_BOOL crb_tree_delete(CRB_TREE *crbtree, CRB_NODE *node)
{
    __crb_tree_erase(crbtree, node);

    CRB_TREE_DATA_FREE(crbtree)(CRB_NODE_DATA(node));
    crb_node_free(node);
    
    CRB_TREE_NODE_NUM(crbtree) --;
    
    return (EC_TRUE);
}

CRB_NODE *crb_tree_search_data(const CRB_TREE *crbtree, const void *data)
{
    CRB_NODE *node;

    node = CRB_TREE_ROOT(crbtree);
    
    while (NULL_PTR != node)
    {        
        int cmp;

        cmp = CRB_TREE_DATA_CMP(crbtree)(CRB_NODE_DATA(node), data);
        if (0 < cmp) /*data < CRB_NODE_DATA(node)*/
        {
            node = CRB_NODE_LEFT(node);
        }
        else if (0 > cmp)/*data > CRB_NODE_DATA(node)*/
        {
            node = CRB_NODE_RIGHT(node);
        }
        else
        {
            return (node);
        }
    }

    return (NULL_PTR);
}

CRB_NODE * crb_tree_insert_data(CRB_TREE *crbtree, const void *data)
{
    CRB_NODE *node_tmp;
    CRB_NODE *node_new;

    node_tmp = crb_node_new();
    if(NULL_PTR == node_tmp)
    {
        sys_log(LOGSTDOUT, "error:crb_tree_insert_data: new node failed\n");
        return (NULL_PTR);
    }

    CRB_NODE_DATA(node_tmp) = (void *)data;
    
    node_new = crb_tree_insert(crbtree, node_tmp);
    if(node_new != node_tmp)
    {
        crb_node_free(node_tmp);
    }

    return (node_new);
}

EC_BOOL crb_tree_delete_data(CRB_TREE *crbtree, const void *data)
{
    CRB_NODE *node;

    node = crb_tree_search_data(crbtree, data);
    if(NULL_PTR == node)
    {
        return (EC_TRUE);
    }

    return crb_tree_delete(crbtree, node);
}


/*postorder: left -> right -> root*/
static void __crb_tree_free(CRB_TREE *crbtree, CRB_NODE *node)
{
    if(NULL_PTR == node)
    {
        return;
    }
    
    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        __crb_tree_free(crbtree, CRB_NODE_LEFT(node));
    }    

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        __crb_tree_free(crbtree, CRB_NODE_RIGHT(node));
    }    

    CRB_TREE_DATA_FREE(crbtree)(CRB_NODE_DATA(node));
    crb_node_free(node);
    CRB_TREE_NODE_NUM(crbtree) --;
    
    return;
}

CRB_TREE *crb_tree_new(CRB_DATA_CMP data_cmp, CRB_DATA_FREE data_free, CRB_DATA_PRINT data_print)
{    
    CRB_TREE *crbtree;

    alloc_static_mem(MD_TBD, ERR_MODULE_ID, MM_CRB_TREE, &crbtree, LOC_CRB_0003);
    if(NULL_PTR == crbtree)
    {
        sys_log(LOGSTDOUT, "error:crb_tree_new: new crbtree failed\n");
        return (NULL_PTR);
    }   
    crb_tree_init(crbtree, data_cmp, data_free, data_print);
    return (crbtree);
}

EC_BOOL crb_tree_init(CRB_TREE *crbtree, CRB_DATA_CMP data_cmp, CRB_DATA_FREE data_free, CRB_DATA_PRINT data_print)
{
    CRB_TREE_NODE_NUM(crbtree)    = 0;
    CRB_TREE_ROOT(crbtree)        = NULL_PTR;
    CRB_TREE_DATA_CMP(crbtree)    = (NULL_PTR == data_cmp)   ? __crb_data_cmp_default   : data_cmp;
    CRB_TREE_DATA_FREE(crbtree)   = (NULL_PTR == data_free)  ? __crb_data_null_default  : data_free;
    CRB_TREE_DATA_PRINT(crbtree)  = (NULL_PTR == data_print) ? __crb_data_print_default : data_print;
    
    return (EC_TRUE);
}

void crb_tree_clean(CRB_TREE *crbtree)
{ 
    if(NULL_PTR != crbtree)
    {
        __crb_tree_free(crbtree, CRB_TREE_ROOT(crbtree));
        CRB_TREE_ROOT(crbtree) = NULL_PTR;
    }
    return;
}

void crb_tree_free(CRB_TREE *crbtree)
{
    if(NULL_PTR != crbtree)
    {
        crb_tree_clean(crbtree);
        free_static_mem(MD_TBD, ERR_MODULE_ID, MM_CRB_TREE, crbtree, LOC_CRB_0004);
    }
    return;
}

void crb_tree_print(LOG *log, const CRB_TREE *crbtree)
{
    sys_log(log, "crbtree %lx, node_num %u\n", 
                 crbtree, 
                 CRB_TREE_NODE_NUM(crbtree));

    crb_inorder_print(log, crbtree);
    return;
}

void crb_tree_print_in_order(LOG *log, const CRB_TREE *crbtree)
{
    const CRB_NODE *node;

    sys_log(log, "[root = %p]\n", CRB_TREE_ROOT(crbtree));
    
    for(node = crb_tree_first_node(crbtree); NULL_PTR != node; node = crb_tree_next_node(crbtree, node))
    {
        crb_node_print(log, crbtree, node);
    }
    return;
}

/*visit the root node first: root -> left -> right*/
static void __crb_preorder_print(LOG *log, const CRB_TREE *crbtree, const CRB_NODE *node)
{
    if(NULL_PTR == node)
    {
        return;
    }
    
    crb_node_print(log, crbtree, node);

    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        __crb_preorder_print(log, crbtree, CRB_NODE_LEFT(node));
    }

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        __crb_preorder_print(log, crbtree, CRB_NODE_RIGHT(node));
    }    
    
    return;
}

void crb_preorder_print(LOG *log, const CRB_TREE *crbtree)
{
    __crb_preorder_print(log, crbtree, CRB_TREE_ROOT(crbtree));
    return;
}

/*visit the left subtree, then the root node: left -> root -> right*/
static void __crb_inorder_print(LOG *log, const CRB_TREE *crbtree, const CRB_NODE *node)
{
    if(NULL_PTR == node)
    {
        return;
    }
    
    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        __crb_inorder_print(log, crbtree, CRB_NODE_LEFT(node));
    }

    crb_node_print(log, crbtree, node);

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        __crb_inorder_print(log, crbtree, CRB_NODE_RIGHT(node));
    }    
    
    return;
}

void crb_inorder_print(LOG *log, const CRB_TREE *crbtree)
{
    __crb_inorder_print(log, crbtree, CRB_TREE_ROOT(crbtree));
    return;
}

/*visit the root node last: left -> right -> root*/
static void __crb_postorder_print(LOG *log, const CRB_TREE *crbtree, const CRB_NODE *node)
{
    if(NULL_PTR == node)
    {
        return;
    }
    
    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        __crb_postorder_print(log, crbtree, CRB_NODE_LEFT(node));
    }    

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        __crb_postorder_print(log, crbtree, CRB_NODE_RIGHT(node));
    }    

    crb_node_print(log, crbtree, node);
    
    return;
}

void crb_postorder_print(LOG *log, const CRB_TREE *crbtree)
{
    __crb_postorder_print(log, crbtree, CRB_TREE_ROOT(crbtree));
    return;
}

/*visit the root node first: root -> left -> right*/
static void __crb_preorder_print_level(LOG *log, const CRB_TREE *crbtree, const CRB_NODE *node, const uint16_t level)
{
    if(NULL_PTR == node)
    {
        return;
    }
    
    crb_node_print_level(log, crbtree, node, level);

    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        __crb_preorder_print_level(log, crbtree, CRB_NODE_LEFT(node), level + 1);
    }

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        __crb_preorder_print_level(log, crbtree, CRB_NODE_RIGHT(node), level + 1);
    }    
    
    return;
}

void crb_preorder_print_level(LOG *log, const CRB_TREE *crbtree, const uint16_t level)
{
    __crb_preorder_print_level(log, crbtree, CRB_TREE_ROOT(crbtree), level);
    return;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
