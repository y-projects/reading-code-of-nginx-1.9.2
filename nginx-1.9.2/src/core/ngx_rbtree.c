
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * The red-black tree code is based on the algorithm described in
 * the "Introduction to Algorithms" by Cormen, Leiserson and Rivest.
 */


static ngx_inline void ngx_rbtree_left_rotate(ngx_rbtree_node_t **root,
    ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);
static ngx_inline void ngx_rbtree_right_rotate(ngx_rbtree_node_t **root,
    ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);

/*
��7-4 NginxΪ������Ѿ�ʵ�ֺõ�3���������ӷ��� (ngx_rbtree_insert_ptָ���������ַ���)
���������������������������������������ש��������������������������������������ש�������������������������������
��    ������                          ��    ��������                          ��    ִ������                  ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_value        ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     �����ݽڵ�Ĺؼ��ֶ���Ψһ�ģ�  ��
��ngx_rbtree_node_t *node,            ����ָ�룻sentinel����ú������ʼ��    ��������ͬһ���ؼ����ж���ڵ�  ��
��ngx_rbtree_node_t *sentinel)        ��ʱ�ڱ��ڵ��ָ��                      ��������                        ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_timer_value  ��  root�Ǻ����������ָ�룻node��      ��                              ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     ��  �������������ݽڵ㣬ÿ��  ��
��ngx_rbtree_node_t *node,            ����ָ�룬����Ӧ�Ĺؼ�����ʱ�����      �����ݽڵ�Ĺؼ��ֱ�ʾʱ������  ��
��                                    ��ʱ�������Ǹ�����sentinel�����    ��ʱ���                        ��
��ngx_rbtree_node_t *sentinel)        ��                                      ��                              ��
��                                    ���������ʼ��ʱ���ڱ��ڵ�              ��                              ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_str_rbtree_insert_value    ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *temp,           ��������Ԫ�ص�ngx_str_node_t��Ա��      �����ݽڵ�Ĺؼ��ֿ��Բ���Ψһ  ��
��ngx_rbtree_node_t *node,            ��ָ�루ngx- rbtree_node_t���ͻ�ǿ��ת  ���ģ������������ַ�����ΪΨһ  ��
��                                    ����Ϊngx_str_node_t���ͣ���sentinel��  ���ı�ʶ�������ngx_str_node_t  ��
��ngx_rbtree_node t *sentinel)        ��                                      ��                              ��
��                                    ����ú������ʼ��ʱ�ڱ��ڵ��ָ��      ���ṹ���str��Ա��             ��
���������������������������������������ߩ��������������������������������������ߩ�������������������������������
    ͬʱ������ngx_str_node_t�ڵ㣬Nginx���ṩ��ngx_str_rbtree_lookup�������ڼ���
������ڵ㣬��������һ�����Ķ��壬�������¡�
    ngx_str_node_t  *ngx_str_rbtree_lookup(ngx_rbtree t  *rbtree,  ngx_str_t *name, uint32_t hash)��
    ���У�hash������Ҫ��ѯ�ڵ��key�ؼ��֣���name��Ҫ��ѯ���ַ����������ͬ��
������Ӧ��ͬkey�ؼ��ֵ����⣩�����ص��ǲ�ѯ���ĺ�����ڵ�ṹ�塣
    ���ں���������ķ�������7-5��
��7-5  ����������ṩ�ķ���
���������������������������������������ש��������������������������������ש�������������������������������������
��    ������                          ��    ��������                    ��    ִ������                        ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��                                    ��  tree�Ǻ����������ָ�룻s��   ��  ��ʼ���������������ʼ������      ��
��                                    ���ڱ��ڵ��ָ�룻i��ngx_rbtree_  ��                                    ��
��ngx_rbtree_init(tree, s, i)         ��                                ���㡢�ڱ��ڵ㡢ngx_rbtree_insert_pt  ��
��                                    ��insert_pt���͵Ľڵ����ӷ������� ���ڵ����ӷ���                        ��
��                                    �������7-4                       ��                                    ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_insert(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �����������ӽڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ������Ҫ���ӵ�������Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_delete(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �Ӻ������ɾ���ڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ���Ǻ��������Ҫɾ���Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
���������������������������������������ߩ��������������������������������ߩ�������������������������������������
    �ڳ�ʼ�������ʱ����Ҫ�ȷ���ñ���������ngx_rbtree_t�ṹ�壬�Լ�ngx_rbtree_
node_t���͵��ڱ��ڵ㣬��ѡ������Զ���ngx_rbtree_insert_pt���͵Ľڵ����Ӻ�����
    ���ں������ÿ���ڵ���˵�����Ƕ��߱���7-6���е�7�����������ֻ�����˽����
ʹ�ú��������ôֻ��Ҫ�˽�ngx_rbtree_min������


��7��Nginx�ṩ�ĸ߼����ݽṹר233
��7-6������ڵ��ṩ�ķ���
���������������������������������������ש����������������������������������ש���������������������������������������
��    ������                          ��    ��������                      ��    ִ������                          ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_red(node)                   ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    �� t���͵Ľڵ�ָ��                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_black(node)                 ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    ��t���͵Ľڵ�ָ��                   ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�O   ��
��ngx_rbt_is_red(node)                ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbt_is_black(node)              ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�0   ��
��                        I           ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��               I                    ��  nl��n2���Ǻ������ngx_rbtree_   ��                                      ��
��ngx_rbt_copy_color(nl, n2)          ��                                  ��  ��n2�ڵ����ɫ���Ƶ�nl�ڵ�          ��
��                                 I  ��nodej���͵Ľڵ�ָ��               ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbtree_node_t *                 ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbtree_min                      ��t���͵Ľڵ�ָ�룻sentinel����ú� ��  �ҵ���ǰ�ڵ㼰�������е���С�ڵ�    ��
��(ngx_rbtree_node_tľnode,           ���������ڱ��ڵ�                    ��������key�ؼ��֣�                     ��
��ngx_rbtree_node_t *sentinel)        ��                                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��ʼ���ڱ��ڵ㣬ʵ���Ͼ��ǽ��ýڵ�  ��
��ngx_rbtree_sentinel_init(node)      ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ɫ��Ϊ��ɫ                          ��
���������������������������������������ߩ����������������������������������ߩ���������������������������������������
    ��7-5�еķ����󲿷�����ʵ�ֻ�����չ������Ĺ��ܣ����ֻ��ʹ�ú��������ôһ
�������ֻ��ʹ��ngx_rbtre e_min������
    ���ڽ��ܵķ������߽ṹ��ļ��÷���ʵ�ֿɲμ�7.5.4�ڵ����ʾ����


ʹ�ú�����ļ�����
    ������һ���򵥵�������˵�����ʹ�ú����������������ջ�з���rbtree���������
�ṹ���Լ��ڱ��ڵ�sentinel����Ȼ��Ҳ����ʹ���ڴ�ػ��ߴӽ��̶��з��䣩�������еĽ�
����ȫ��key�ؼ�����Ϊÿ���ڵ��Ψһ��ʶ�������Ϳ��Բ���Ԥ���ngx_rbtree insert
value�����ˡ����ɵ���ngx_rbtree_init������ʼ�������������������ʾ��
    ngx_rbtree_node_t  sentinel ;
    ngx_rbtree_init ( &rbtree, &sentinel,ngx_str_rbtree_insert_value)
    ���������ڵ�Ľṹ�彫ʹ��7.5.3���н��ܵ�TestRBTreeNode�ṹ�壬���е����н�
�㶼ȡ��ͼ7-7��ÿ��Ԫ�ص�key�ؼ��ְ���1��6��8��11��13��15��17��22��25��27��˳
��һһ�����������ӣ�����������ʾ��
    rbTreeNode [0] .num=17;
    rbTreeNode [1] .num=22;
    rbTreeNode [2] .num=25;
    rbTreeNode [3] .num=27;
    rbTreeNode [4] .num=17;
    rbTreeNode [7] .num=22;
    rbTreeNode [8] .num=25;
    rbTreeNode [9] .num=27;
    for(i=0j i<10; i++)
    {
        rbTreeNode [i].node. key=rbTreeNode[i]. num;
        ngx_rbtree_insert(&rbtree,&rbTreeNode[i].node);
    )
    ������˳��������ĺ������̬��ͼ7-7��ʾ�������Ҫ�ҳ���ǰ���������С�Ľ�
�㣬���Ե���ngx_rbtree_min������ȡ��
ngx_rbtree_node_t *tmpnode   =   ngx_rbtree_min ( rbtree . root ,    &sentinel )  ;
    ��Ȼ�������������ʹ�ø��ڵ����ʹ����һ���ڵ�Ҳ�ǿ��Եġ���������һ�����
����1���ڵ㣬��ȻNginx�Դ˲�û���ṩԤ��ķ����������ַ��������ṩ��ngx_str_
rbtree_lookup��������������ʵ���ϼ����Ƿǳ��򵥵ġ�������Ѱ��key�ؼ���Ϊ13�Ľڵ�
Ϊ��������˵����
    ngx_uint_t lookupkey=13;
    tmpnode=rbtree.root;
    TestRBTreeNode *lookupNode;
    while (tmpnode  !=&sentinel)  {
        if (lookupkey!-tmpnode->key)  (
        ��������key�ؼ����뵱ǰ�ڵ�Ĵ�С�Ƚϣ������Ǽ�������������������
        tmpnode=  (lookupkey<tmpnode->key)  ?tmpnode->left:tmpnode->right;
        continue��
        )
        �����ҵ���ֵΪ13�����ڵ�
        lookupNode=  (TestRBTreeNode*)  tmpnode;
        break;
    )
    �Ӻ������ɾ��1���ڵ�Ҳ�Ƿǳ��򵥵ģ���Ѹո��ҵ���ֵΪ13�Ľڵ��rbtree��
ɾ����ֻ�����ngx_rbtree_delete������
ngx_rbtree_delete ( &rbtree , &lookupNode->node);

7.5.5����Զ������ӳ�Ա����
    ���ڽڵ��key�ؼ��ֱ��������ͣ��⵼�ºܶ�����²�ͬ�Ľڵ�������ͬ��key��
���֡������ϣ�����־�����ͬkey�ؼ��ֵĲ�ͬ�ڵ�������������ʱ���ָ���ԭ�ڵ��
���������Ҫʵ�����е�ngx_rbtree_insert_ptܵ����
    ����Nginxģ����ʹ�ú����ʱ���Զ�����ngx_rbtree_insert_pt��������geo��
filecacheģ��ȣ���������7.5.3���н��ܹ���ngx_str_rbtree insert valueΪ������˵�����





��7��Nginx�ṩ�ĸ߼����ݽṹ��235
���������ķ������ȿ�һ��ngx_str_rbtree insert value��ʵ�֡��������¡�
void :
       I
ngx_str_rbtree_insert_value  ( ngx_rbt ree_node_t    * temp ,
           -l-
     n,gx_rbtree_node_t *node, ngx_rbtree_node t *sentinel)
                                                                                               .
    n:,gx_str_node_t            *n,   ^ t ;
     jgx_rbtree node t :k*p;
l,  n = (ngx_str_node_t *) node;
   I  t = (ngx_str_node t k) temp;
    �������ȱȽ�key�ؼ��֣����������key��Ϊ��һ�����ؼ���
    if  (node- >key!-temp- >key)  {
    �����������ڵ�Ĺؼ���С��������
    p=  (node->key<temp->key)  ?&temp->left  :  &temp->right;
    )
    ������key�ؼ�����ͬʱ�����ַ�������Ϊ�ڶ������ؼ���
    else if  (n- >str. len!_t->str.len)  (
    һ  �����������ڵ��ַ����ĳ���С��������
    p=(n->str. len<t->str. len)  ?&temp- >left:&temp->rightj
    )   else{
    ����key�ؼ�����ͬ���ַ���������ͬʱ���ټ����Ƚ��ַ�������
    p=  (ngx_memcmp (n->str.data,t->str.data,n->str. len)<0)?&temp- >left
&temp - >right j
    )
���������ǰ�ڵ�p���ڱ��ڵ㣬��ô����ѭ��׼������ڵ�
if (*p==sentinel)  {
    break��
)
//p�ڵ���Ҫ����Ľڵ������ͬ�ı�ʶ��ʱ�����븲������
temp=*p;
    *;p=node;
    �����ò���ڵ�ĸ��ڵ�
    node- >parent=temp;
    I�������ӽڵ㶼���ڱ��ڵ�
    node->left=sentinel;
    nOdeһ>righ��=8en��inel��
    ��+���ڵ���ɫ��Ϊ��ɫ��ע�⣬�������ngx-һrbtree insert�������ڿ��ܵ���ת���������øýڵ�
����ɫ+��
    ng��(һrb��һred(node)��
    )  1
    ���Կ������ô�����7.5.4���н��ܹ��ļ����ڵ��������ơ�����Ҫ��������Ҫ��
����ǵ�key�ؼ�����ͬʱ�������Ժ������ݽṹ��Ϊ��׼��ȷ��������ڵ��Ψһ�ԡ�
Nginx���Ѿ�ʵ�ֵ����ngx_rbtree_insert_pt����Ƿǳ����Ƶģ�������ȫ���Բ���ngx_




236��ڶ�������α�дHTTPģ��
str rbtree insert value�������Զ��������ڵ����ӷ�����

*/

void
ngx_rbtree_insert(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{
    ngx_rbtree_node_t  **root, *temp, *sentinel;

    /* a binary tree insert */

    root = (ngx_rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    if (*root == sentinel) {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        ngx_rbt_black(node);
        *root = node;

        return;
    }

    tree->insert(*root, node, sentinel);

    /* re-balance tree */

    while (node != *root && ngx_rbt_is_red(node->parent)) {

        if (node->parent == node->parent->parent->left) {
            temp = node->parent->parent->right;

            if (ngx_rbt_is_red(temp)) {
                ngx_rbt_black(node->parent);
                ngx_rbt_black(temp);
                ngx_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    ngx_rbtree_left_rotate(root, sentinel, node);
                }

                ngx_rbt_black(node->parent);
                ngx_rbt_red(node->parent->parent);
                ngx_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            temp = node->parent->parent->left;

            if (ngx_rbt_is_red(temp)) {
                ngx_rbt_black(node->parent);
                ngx_rbt_black(temp);
                ngx_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    ngx_rbtree_right_rotate(root, sentinel, node);
                }

                ngx_rbt_black(node->parent);
                ngx_rbt_red(node->parent->parent);
                ngx_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }

    ngx_rbt_black(*root);
}

/*
��7-4 NginxΪ������Ѿ�ʵ�ֺõ�3���������ӷ��� (ngx_rbtree_insert_ptָ���������ַ���)
���������������������������������������ש��������������������������������������ש�������������������������������
��    ������                          ��    ��������                          ��    ִ������                  ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_value        ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     �����ݽڵ�Ĺؼ��ֶ���Ψһ�ģ�  ��
��ngx_rbtree_node_t *node,            ����ָ�룻sentinel����ú������ʼ��    ��������ͬһ���ؼ����ж���ڵ�  ��
��ngx_rbtree_node_t *sentinel)        ��ʱ�ڱ��ڵ��ָ��                      ��������                        ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_timer_value  ��  root�Ǻ����������ָ�룻node��      ��                              ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     ��  �������������ݽڵ㣬ÿ��  ��
��ngx_rbtree_node_t *node,            ����ָ�룬����Ӧ�Ĺؼ�����ʱ�����      �����ݽڵ�Ĺؼ��ֱ�ʾʱ������  ��
��                                    ��ʱ�������Ǹ�����sentinel�����    ��ʱ���                        ��
��ngx_rbtree_node_t *sentinel)        ��                                      ��                              ��
��                                    ���������ʼ��ʱ���ڱ��ڵ�              ��                              ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_str_rbtree_insert_value    ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *temp,           ��������Ԫ�ص�ngx_str_node_t��Ա��      �����ݽڵ�Ĺؼ��ֿ��Բ���Ψһ  ��
��ngx_rbtree_node_t *node,            ��ָ�루ngx- rbtree_node_t���ͻ�ǿ��ת  ���ģ������������ַ�����ΪΨһ  ��
��                                    ����Ϊngx_str_node_t���ͣ���sentinel��  ���ı�ʶ�������ngx_str_node_t  ��
��ngx_rbtree_node t *sentinel)        ��                                      ��                              ��
��                                    ����ú������ʼ��ʱ�ڱ��ڵ��ָ��      ���ṹ���str��Ա��             ��
���������������������������������������ߩ��������������������������������������ߩ�������������������������������
    ͬʱ������ngx_str_node_t�ڵ㣬Nginx���ṩ��ngx_str_rbtree_lookup�������ڼ���
������ڵ㣬��������һ�����Ķ��壬�������¡�
    ngx_str_node_t  *ngx_str_rbtree_lookup(ngx_rbtree t  *rbtree,  ngx_str_t *name, uint32_t hash)��
    ���У�hash������Ҫ��ѯ�ڵ��key�ؼ��֣���name��Ҫ��ѯ���ַ����������ͬ��
������Ӧ��ͬkey�ؼ��ֵ����⣩�����ص��ǲ�ѯ���ĺ�����ڵ�ṹ�塣
    ���ں���������ķ�������7-5��
��7-5  ����������ṩ�ķ���
���������������������������������������ש��������������������������������ש�������������������������������������
��    ������                          ��    ��������                    ��    ִ������                        ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��                                    ��  tree�Ǻ����������ָ�룻s��   ��  ��ʼ���������������ʼ������      ��
��                                    ���ڱ��ڵ��ָ�룻i��ngx_rbtree_  ��                                    ��
��ngx_rbtree_init(tree, s, i)         ��                                ���㡢�ڱ��ڵ㡢ngx_rbtree_insert_pt  ��
��                                    ��insert_pt���͵Ľڵ����ӷ������� ���ڵ����ӷ���                        ��
��                                    �������7-4                       ��                                    ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_insert(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �����������ӽڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ������Ҫ���ӵ�������Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_delete(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �Ӻ������ɾ���ڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ���Ǻ��������Ҫɾ���Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
���������������������������������������ߩ��������������������������������ߩ�������������������������������������
    �ڳ�ʼ�������ʱ����Ҫ�ȷ���ñ���������ngx_rbtree_t�ṹ�壬�Լ�ngx_rbtree_
node_t���͵��ڱ��ڵ㣬��ѡ������Զ���ngx_rbtree_insert_pt���͵Ľڵ����Ӻ�����
    ���ں������ÿ���ڵ���˵�����Ƕ��߱���7-6���е�7�����������ֻ�����˽����
ʹ�ú��������ôֻ��Ҫ�˽�ngx_rbtree_min������


��7��Nginx�ṩ�ĸ߼����ݽṹר233
��7-6������ڵ��ṩ�ķ���
���������������������������������������ש����������������������������������ש���������������������������������������
��    ������                          ��    ��������                      ��    ִ������                          ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_red(node)                   ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    �� t���͵Ľڵ�ָ��                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_black(node)                 ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    ��t���͵Ľڵ�ָ��                   ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�O   ��
��ngx_rbt_is_red(node)                ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbt_is_black(node)              ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�0   ��
��                        I           ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��               I                    ��  nl��n2���Ǻ������ngx_rbtree_   ��                                      ��
��ngx_rbt_copy_color(nl, n2)          ��                                  ��  ��n2�ڵ����ɫ���Ƶ�nl�ڵ�          ��
��                                 I  ��nodej���͵Ľڵ�ָ��               ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbtree_node_t *                 ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbtree_min                      ��t���͵Ľڵ�ָ�룻sentinel����ú� ��  �ҵ���ǰ�ڵ㼰�������е���С�ڵ�    ��
��(ngx_rbtree_node_tľnode,           ���������ڱ��ڵ�                    ��������key�ؼ��֣�                     ��
��ngx_rbtree_node_t *sentinel)        ��                                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��ʼ���ڱ��ڵ㣬ʵ���Ͼ��ǽ��ýڵ�  ��
��ngx_rbtree_sentinel_init(node)      ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ɫ��Ϊ��ɫ                          ��
���������������������������������������ߩ����������������������������������ߩ���������������������������������������
    ��7-5�еķ����󲿷�����ʵ�ֻ�����չ������Ĺ��ܣ����ֻ��ʹ�ú��������ôһ
�������ֻ��ʹ��ngx_rbtre e_min������
    ���ڽ��ܵķ������߽ṹ��ļ��÷���ʵ�ֿɲμ�7.5.4�ڵ����ʾ����


ʹ�ú�����ļ�����
    ������һ���򵥵�������˵�����ʹ�ú����������������ջ�з���rbtree���������
�ṹ���Լ��ڱ��ڵ�sentinel����Ȼ��Ҳ����ʹ���ڴ�ػ��ߴӽ��̶��з��䣩�������еĽ�
����ȫ��key�ؼ�����Ϊÿ���ڵ��Ψһ��ʶ�������Ϳ��Բ���Ԥ���ngx_rbtree insert
value�����ˡ����ɵ���ngx_rbtree_init������ʼ�������������������ʾ��
    ngx_rbtree_node_t  sentinel ;
    ngx_rbtree_init ( &rbtree, &sentinel,ngx_str_rbtree_insert_value)
    ���������ڵ�Ľṹ�彫ʹ��7.5.3���н��ܵ�TestRBTreeNode�ṹ�壬���е����н�
�㶼ȡ��ͼ7-7��ÿ��Ԫ�ص�key�ؼ��ְ���1��6��8��11��13��15��17��22��25��27��˳
��һһ�����������ӣ�����������ʾ��
    rbTreeNode [0] .num=17;
    rbTreeNode [1] .num=22;
    rbTreeNode [2] .num=25;
    rbTreeNode [3] .num=27;
    rbTreeNode [4] .num=17;
    rbTreeNode [7] .num=22;
    rbTreeNode [8] .num=25;
    rbTreeNode [9] .num=27;
    for(i=0j i<10; i++)
    {
        rbTreeNode [i].node. key=rbTreeNode[i]. num;
        ngx_rbtree_insert(&rbtree,&rbTreeNode[i].node);
    )
    ������˳��������ĺ������̬��ͼ7-7��ʾ�������Ҫ�ҳ���ǰ���������С�Ľ�
�㣬���Ե���ngx_rbtree_min������ȡ��
ngx_rbtree_node_t *tmpnode   =   ngx_rbtree_min ( rbtree . root ,    &sentinel )  ;
    ��Ȼ�������������ʹ�ø��ڵ����ʹ����һ���ڵ�Ҳ�ǿ��Եġ���������һ�����
����1���ڵ㣬��ȻNginx�Դ˲�û���ṩԤ��ķ����������ַ��������ṩ��ngx_str_
rbtree_lookup��������������ʵ���ϼ����Ƿǳ��򵥵ġ�������Ѱ��key�ؼ���Ϊ13�Ľڵ�
Ϊ��������˵����
    ngx_uint_t lookupkey=13;
    tmpnode=rbtree.root;
    TestRBTreeNode *lookupNode;
    while (tmpnode  !=&sentinel)  {
        if (lookupkey!-tmpnode->key)  (
        ��������key�ؼ����뵱ǰ�ڵ�Ĵ�С�Ƚϣ������Ǽ�������������������
        tmpnode=  (lookupkey<tmpnode->key)  ?tmpnode->left:tmpnode->right;
        continue��
        )
        �����ҵ���ֵΪ13�����ڵ�
        lookupNode=  (TestRBTreeNode*)  tmpnode;
        break;
    )
    �Ӻ������ɾ��1���ڵ�Ҳ�Ƿǳ��򵥵ģ���Ѹո��ҵ���ֵΪ13�Ľڵ��rbtree��
ɾ����ֻ�����ngx_rbtree_delete������
ngx_rbtree_delete ( &rbtree , &lookupNode->node);

7.5.5����Զ������ӳ�Ա����
    ���ڽڵ��key�ؼ��ֱ��������ͣ��⵼�ºܶ�����²�ͬ�Ľڵ�������ͬ��key��
���֡������ϣ�����־�����ͬkey�ؼ��ֵĲ�ͬ�ڵ�������������ʱ���ָ���ԭ�ڵ��
���������Ҫʵ�����е�ngx_rbtree_insert_ptܵ����
    ����Nginxģ����ʹ�ú����ʱ���Զ�����ngx_rbtree_insert_pt��������geo��
filecacheģ��ȣ���������7.5.3���н��ܹ���ngx_str_rbtree insert valueΪ������˵�����





��7��Nginx�ṩ�ĸ߼����ݽṹ��235
���������ķ������ȿ�һ��ngx_str_rbtree insert value��ʵ�֡��������¡�
void :
       I
ngx_str_rbtree_insert_value  ( ngx_rbt ree_node_t    * temp ,
           -l-
     n,gx_rbtree_node_t *node, ngx_rbtree_node t *sentinel)
                                                                                               .
    n:,gx_str_node_t            *n,   ^ t ;
     jgx_rbtree node t :k*p;
l,  n = (ngx_str_node_t *) node;
   I  t = (ngx_str_node t k) temp;
    �������ȱȽ�key�ؼ��֣����������key��Ϊ��һ�����ؼ���
    if  (node- >key!-temp- >key)  {
    �����������ڵ�Ĺؼ���С��������
    p=  (node->key<temp->key)  ?&temp->left  :  &temp->right;
    )
    ������key�ؼ�����ͬʱ�����ַ�������Ϊ�ڶ������ؼ���
    else if  (n- >str. len!_t->str.len)  (
    һ  �����������ڵ��ַ����ĳ���С��������
    p=(n->str. len<t->str. len)  ?&temp- >left:&temp->rightj
    )   else{
    ����key�ؼ�����ͬ���ַ���������ͬʱ���ټ����Ƚ��ַ�������
    p=  (ngx_memcmp (n->str.data,t->str.data,n->str. len)<0)?&temp- >left
&temp - >right j
    )
���������ǰ�ڵ�p���ڱ��ڵ㣬��ô����ѭ��׼������ڵ�
if (*p==sentinel)  {
    break��
)
//p�ڵ���Ҫ����Ľڵ������ͬ�ı�ʶ��ʱ�����븲������
temp=*p;
    *;p=node;
    �����ò���ڵ�ĸ��ڵ�
    node- >parent=temp;
    I�������ӽڵ㶼���ڱ��ڵ�
    node->left=sentinel;
    nOdeһ>righ��=8en��inel��
    ��+���ڵ���ɫ��Ϊ��ɫ��ע�⣬�������ngx-һrbtree insert�������ڿ��ܵ���ת���������øýڵ�
����ɫ+��
    ng��(һrb��һred(node)��
    )  1
    ���Կ������ô�����7.5.4���н��ܹ��ļ����ڵ��������ơ�����Ҫ��������Ҫ��
����ǵ�key�ؼ�����ͬʱ�������Ժ������ݽṹ��Ϊ��׼��ȷ��������ڵ��Ψһ�ԡ�
Nginx���Ѿ�ʵ�ֵ����ngx_rbtree_insert_pt����Ƿǳ����Ƶģ�������ȫ���Բ���ngx_




236��ڶ�������α�дHTTPģ��
str rbtree insert value�������Զ��������ڵ����ӷ�����

*/

void
ngx_rbtree_insert_value(ngx_rbtree_node_t *temp, ngx_rbtree_node_t *node,
    ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t  **p;

    for ( ;; ) {

        p = (node->key < temp->key) ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}

/*
��7-4 NginxΪ������Ѿ�ʵ�ֺõ�3���������ӷ��� (ngx_rbtree_insert_ptָ���������ַ���)
���������������������������������������ש��������������������������������������ש�������������������������������
��    ������                          ��    ��������                          ��    ִ������                  ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_value        ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     �����ݽڵ�Ĺؼ��ֶ���Ψһ�ģ�  ��
��ngx_rbtree_node_t *node,            ����ָ�룻sentinel����ú������ʼ��    ��������ͬһ���ؼ����ж���ڵ�  ��
��ngx_rbtree_node_t *sentinel)        ��ʱ�ڱ��ڵ��ָ��                      ��������                        ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_timer_value  ��  root�Ǻ����������ָ�룻node��      ��                              ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     ��  �������������ݽڵ㣬ÿ��  ��
��ngx_rbtree_node_t *node,            ����ָ�룬����Ӧ�Ĺؼ�����ʱ�����      �����ݽڵ�Ĺؼ��ֱ�ʾʱ������  ��
��                                    ��ʱ�������Ǹ�����sentinel�����    ��ʱ���                        ��
��ngx_rbtree_node_t *sentinel)        ��                                      ��                              ��
��                                    ���������ʼ��ʱ���ڱ��ڵ�              ��                              ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_str_rbtree_insert_value    ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *temp,           ��������Ԫ�ص�ngx_str_node_t��Ա��      �����ݽڵ�Ĺؼ��ֿ��Բ���Ψһ  ��
��ngx_rbtree_node_t *node,            ��ָ�루ngx- rbtree_node_t���ͻ�ǿ��ת  ���ģ������������ַ�����ΪΨһ  ��
��                                    ����Ϊngx_str_node_t���ͣ���sentinel��  ���ı�ʶ�������ngx_str_node_t  ��
��ngx_rbtree_node t *sentinel)        ��                                      ��                              ��
��                                    ����ú������ʼ��ʱ�ڱ��ڵ��ָ��      ���ṹ���str��Ա��             ��
���������������������������������������ߩ��������������������������������������ߩ�������������������������������
    ͬʱ������ngx_str_node_t�ڵ㣬Nginx���ṩ��ngx_str_rbtree_lookup�������ڼ���
������ڵ㣬��������һ�����Ķ��壬�������¡�
    ngx_str_node_t  *ngx_str_rbtree_lookup(ngx_rbtree t  *rbtree,  ngx_str_t *name, uint32_t hash)��
    ���У�hash������Ҫ��ѯ�ڵ��key�ؼ��֣���name��Ҫ��ѯ���ַ����������ͬ��
������Ӧ��ͬkey�ؼ��ֵ����⣩�����ص��ǲ�ѯ���ĺ�����ڵ�ṹ�塣
    ���ں���������ķ�������7-5��
��7-5  ����������ṩ�ķ���
���������������������������������������ש��������������������������������ש�������������������������������������
��    ������                          ��    ��������                    ��    ִ������                        ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��                                    ��  tree�Ǻ����������ָ�룻s��   ��  ��ʼ���������������ʼ������      ��
��                                    ���ڱ��ڵ��ָ�룻i��ngx_rbtree_  ��                                    ��
��ngx_rbtree_init(tree, s, i)         ��                                ���㡢�ڱ��ڵ㡢ngx_rbtree_insert_pt  ��
��                                    ��insert_pt���͵Ľڵ����ӷ������� ���ڵ����ӷ���                        ��
��                                    �������7-4                       ��                                    ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_insert(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �����������ӽڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ������Ҫ���ӵ�������Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_delete(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �Ӻ������ɾ���ڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ���Ǻ��������Ҫɾ���Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
���������������������������������������ߩ��������������������������������ߩ�������������������������������������
    �ڳ�ʼ�������ʱ����Ҫ�ȷ���ñ���������ngx_rbtree_t�ṹ�壬�Լ�ngx_rbtree_
node_t���͵��ڱ��ڵ㣬��ѡ������Զ���ngx_rbtree_insert_pt���͵Ľڵ����Ӻ�����
    ���ں������ÿ���ڵ���˵�����Ƕ��߱���7-6���е�7�����������ֻ�����˽����
ʹ�ú��������ôֻ��Ҫ�˽�ngx_rbtree_min������


��7��Nginx�ṩ�ĸ߼����ݽṹר233
��7-6������ڵ��ṩ�ķ���
���������������������������������������ש����������������������������������ש���������������������������������������
��    ������                          ��    ��������                      ��    ִ������                          ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_red(node)                   ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    �� t���͵Ľڵ�ָ��                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_black(node)                 ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    ��t���͵Ľڵ�ָ��                   ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�O   ��
��ngx_rbt_is_red(node)                ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbt_is_black(node)              ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�0   ��
��                        I           ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��               I                    ��  nl��n2���Ǻ������ngx_rbtree_   ��                                      ��
��ngx_rbt_copy_color(nl, n2)          ��                                  ��  ��n2�ڵ����ɫ���Ƶ�nl�ڵ�          ��
��                                 I  ��nodej���͵Ľڵ�ָ��               ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbtree_node_t *                 ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbtree_min                      ��t���͵Ľڵ�ָ�룻sentinel����ú� ��  �ҵ���ǰ�ڵ㼰�������е���С�ڵ�    ��
��(ngx_rbtree_node_tľnode,           ���������ڱ��ڵ�                    ��������key�ؼ��֣�                     ��
��ngx_rbtree_node_t *sentinel)        ��                                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��ʼ���ڱ��ڵ㣬ʵ���Ͼ��ǽ��ýڵ�  ��
��ngx_rbtree_sentinel_init(node)      ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ɫ��Ϊ��ɫ                          ��
���������������������������������������ߩ����������������������������������ߩ���������������������������������������
    ��7-5�еķ����󲿷�����ʵ�ֻ�����չ������Ĺ��ܣ����ֻ��ʹ�ú��������ôһ
�������ֻ��ʹ��ngx_rbtre e_min������
    ���ڽ��ܵķ������߽ṹ��ļ��÷���ʵ�ֿɲμ�7.5.4�ڵ����ʾ����


ʹ�ú�����ļ�����
    ������һ���򵥵�������˵�����ʹ�ú����������������ջ�з���rbtree���������
�ṹ���Լ��ڱ��ڵ�sentinel����Ȼ��Ҳ����ʹ���ڴ�ػ��ߴӽ��̶��з��䣩�������еĽ�
����ȫ��key�ؼ�����Ϊÿ���ڵ��Ψһ��ʶ�������Ϳ��Բ���Ԥ���ngx_rbtree insert
value�����ˡ����ɵ���ngx_rbtree_init������ʼ�������������������ʾ��
    ngx_rbtree_node_t  sentinel ;
    ngx_rbtree_init ( &rbtree, &sentinel,ngx_str_rbtree_insert_value)
    ���������ڵ�Ľṹ�彫ʹ��7.5.3���н��ܵ�TestRBTreeNode�ṹ�壬���е����н�
�㶼ȡ��ͼ7-7��ÿ��Ԫ�ص�key�ؼ��ְ���1��6��8��11��13��15��17��22��25��27��˳
��һһ�����������ӣ�����������ʾ��
    rbTreeNode [0] .num=17;
    rbTreeNode [1] .num=22;
    rbTreeNode [2] .num=25;
    rbTreeNode [3] .num=27;
    rbTreeNode [4] .num=17;
    rbTreeNode [7] .num=22;
    rbTreeNode [8] .num=25;
    rbTreeNode [9] .num=27;
    for(i=0j i<10; i++)
    {
        rbTreeNode [i].node. key=rbTreeNode[i]. num;
        ngx_rbtree_insert(&rbtree,&rbTreeNode[i].node);
    )
    ������˳��������ĺ������̬��ͼ7-7��ʾ�������Ҫ�ҳ���ǰ���������С�Ľ�
�㣬���Ե���ngx_rbtree_min������ȡ��
ngx_rbtree_node_t *tmpnode   =   ngx_rbtree_min ( rbtree . root ,    &sentinel )  ;
    ��Ȼ�������������ʹ�ø��ڵ����ʹ����һ���ڵ�Ҳ�ǿ��Եġ���������һ�����
����1���ڵ㣬��ȻNginx�Դ˲�û���ṩԤ��ķ����������ַ��������ṩ��ngx_str_
rbtree_lookup��������������ʵ���ϼ����Ƿǳ��򵥵ġ�������Ѱ��key�ؼ���Ϊ13�Ľڵ�
Ϊ��������˵����
    ngx_uint_t lookupkey=13;
    tmpnode=rbtree.root;
    TestRBTreeNode *lookupNode;
    while (tmpnode  !=&sentinel)  {
        if (lookupkey!-tmpnode->key)  (
        ��������key�ؼ����뵱ǰ�ڵ�Ĵ�С�Ƚϣ������Ǽ�������������������
        tmpnode=  (lookupkey<tmpnode->key)  ?tmpnode->left:tmpnode->right;
        continue��
        )
        �����ҵ���ֵΪ13�����ڵ�
        lookupNode=  (TestRBTreeNode*)  tmpnode;
        break;
    )
    �Ӻ������ɾ��1���ڵ�Ҳ�Ƿǳ��򵥵ģ���Ѹո��ҵ���ֵΪ13�Ľڵ��rbtree��
ɾ����ֻ�����ngx_rbtree_delete������
ngx_rbtree_delete ( &rbtree , &lookupNode->node);

7.5.5����Զ������ӳ�Ա����
    ���ڽڵ��key�ؼ��ֱ��������ͣ��⵼�ºܶ�����²�ͬ�Ľڵ�������ͬ��key��
���֡������ϣ�����־�����ͬkey�ؼ��ֵĲ�ͬ�ڵ�������������ʱ���ָ���ԭ�ڵ��
���������Ҫʵ�����е�ngx_rbtree_insert_ptܵ����
    ����Nginxģ����ʹ�ú����ʱ���Զ�����ngx_rbtree_insert_pt��������geo��
filecacheģ��ȣ���������7.5.3���н��ܹ���ngx_str_rbtree insert valueΪ������˵�����





��7��Nginx�ṩ�ĸ߼����ݽṹ��235
���������ķ������ȿ�һ��ngx_str_rbtree insert value��ʵ�֡��������¡�
void :
       I
ngx_str_rbtree_insert_value  ( ngx_rbt ree_node_t    * temp ,
           -l-
     n,gx_rbtree_node_t *node, ngx_rbtree_node t *sentinel)
                                                                                               .
    n:,gx_str_node_t            *n,   ^ t ;
     jgx_rbtree node t :k*p;
l,  n = (ngx_str_node_t *) node;
   I  t = (ngx_str_node t k) temp;
    �������ȱȽ�key�ؼ��֣����������key��Ϊ��һ�����ؼ���
    if  (node- >key!-temp- >key)  {
    �����������ڵ�Ĺؼ���С��������
    p=  (node->key<temp->key)  ?&temp->left  :  &temp->right;
    )
    ������key�ؼ�����ͬʱ�����ַ�������Ϊ�ڶ������ؼ���
    else if  (n- >str. len!_t->str.len)  (
    һ  �����������ڵ��ַ����ĳ���С��������
    p=(n->str. len<t->str. len)  ?&temp- >left:&temp->rightj
    )   else{
    ����key�ؼ�����ͬ���ַ���������ͬʱ���ټ����Ƚ��ַ�������
    p=  (ngx_memcmp (n->str.data,t->str.data,n->str. len)<0)?&temp- >left
&temp - >right j
    )
���������ǰ�ڵ�p���ڱ��ڵ㣬��ô����ѭ��׼������ڵ�
if (*p==sentinel)  {
    break��
)
//p�ڵ���Ҫ����Ľڵ������ͬ�ı�ʶ��ʱ�����븲������
temp=*p;
    *;p=node;
    �����ò���ڵ�ĸ��ڵ�
    node- >parent=temp;
    I�������ӽڵ㶼���ڱ��ڵ�
    node->left=sentinel;
    nOdeһ>righ��=8en��inel��
    ��+���ڵ���ɫ��Ϊ��ɫ��ע�⣬�������ngx-һrbtree insert�������ڿ��ܵ���ת���������øýڵ�
����ɫ+��
    ng��(һrb��һred(node)��
    )  1
    ���Կ������ô�����7.5.4���н��ܹ��ļ����ڵ��������ơ�����Ҫ��������Ҫ��
����ǵ�key�ؼ�����ͬʱ�������Ժ������ݽṹ��Ϊ��׼��ȷ��������ڵ��Ψһ�ԡ�
Nginx���Ѿ�ʵ�ֵ����ngx_rbtree_insert_pt����Ƿǳ����Ƶģ�������ȫ���Բ���ngx_




236��ڶ�������α�дHTTPģ��
str rbtree insert value�������Զ��������ڵ����ӷ�����

*/

void
ngx_rbtree_insert_timer_value(ngx_rbtree_node_t *temp, ngx_rbtree_node_t *node,
    ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t  **p;

    for ( ;; ) {

        /*
         * Timer values
         * 1) are spread in small range, usually several minutes,
         * 2) and overflow each 49 days, if milliseconds are stored in 32 bits.
         * The comparison takes into account that overflow.
         */

        /*  node->key < temp->key */

        p = ((ngx_rbtree_key_int_t) (node->key - temp->key) < 0)
            ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}

/*
��7-4 NginxΪ������Ѿ�ʵ�ֺõ�3���������ӷ��� (ngx_rbtree_insert_ptָ���������ַ���)
���������������������������������������ש��������������������������������������ש�������������������������������
��    ������                          ��    ��������                          ��    ִ������                  ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_value        ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     �����ݽڵ�Ĺؼ��ֶ���Ψһ�ģ�  ��
��ngx_rbtree_node_t *node,            ����ָ�룻sentinel����ú������ʼ��    ��������ͬһ���ؼ����ж���ڵ�  ��
��ngx_rbtree_node_t *sentinel)        ��ʱ�ڱ��ڵ��ָ��                      ��������                        ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_rbtree_insert_timer_value  ��  root�Ǻ����������ָ�룻node��      ��                              ��
��(ngx_rbtree_node_t *root,           ��������Ԫ�ص�ngx_rbtree_node_t��Ա     ��  �������������ݽڵ㣬ÿ��  ��
��ngx_rbtree_node_t *node,            ����ָ�룬����Ӧ�Ĺؼ�����ʱ�����      �����ݽڵ�Ĺؼ��ֱ�ʾʱ������  ��
��                                    ��ʱ�������Ǹ�����sentinel�����    ��ʱ���                        ��
��ngx_rbtree_node_t *sentinel)        ��                                      ��                              ��
��                                    ���������ʼ��ʱ���ڱ��ڵ�              ��                              ��
�ǩ������������������������������������贈�������������������������������������贈������������������������������
��void ngx_str_rbtree_insert_value    ��  root�Ǻ����������ָ�룻node��      ��  �������������ݽڵ㣬ÿ��  ��
��(ngx_rbtree_node_t *temp,           ��������Ԫ�ص�ngx_str_node_t��Ա��      �����ݽڵ�Ĺؼ��ֿ��Բ���Ψһ  ��
��ngx_rbtree_node_t *node,            ��ָ�루ngx- rbtree_node_t���ͻ�ǿ��ת  ���ģ������������ַ�����ΪΨһ  ��
��                                    ����Ϊngx_str_node_t���ͣ���sentinel��  ���ı�ʶ�������ngx_str_node_t  ��
��ngx_rbtree_node t *sentinel)        ��                                      ��                              ��
��                                    ����ú������ʼ��ʱ�ڱ��ڵ��ָ��      ���ṹ���str��Ա��             ��
���������������������������������������ߩ��������������������������������������ߩ�������������������������������
    ͬʱ������ngx_str_node_t�ڵ㣬Nginx���ṩ��ngx_str_rbtree_lookup�������ڼ���
������ڵ㣬��������һ�����Ķ��壬�������¡�
    ngx_str_node_t  *ngx_str_rbtree_lookup(ngx_rbtree t  *rbtree,  ngx_str_t *name, uint32_t hash)��
    ���У�hash������Ҫ��ѯ�ڵ��key�ؼ��֣���name��Ҫ��ѯ���ַ����������ͬ��
������Ӧ��ͬkey�ؼ��ֵ����⣩�����ص��ǲ�ѯ���ĺ�����ڵ�ṹ�塣
    ���ں���������ķ�������7-5��
��7-5  ����������ṩ�ķ���
���������������������������������������ש��������������������������������ש�������������������������������������
��    ������                          ��    ��������                    ��    ִ������                        ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��                                    ��  tree�Ǻ����������ָ�룻s��   ��  ��ʼ���������������ʼ������      ��
��                                    ���ڱ��ڵ��ָ�룻i��ngx_rbtree_  ��                                    ��
��ngx_rbtree_init(tree, s, i)         ��                                ���㡢�ڱ��ڵ㡢ngx_rbtree_insert_pt  ��
��                                    ��insert_pt���͵Ľڵ����ӷ������� ���ڵ����ӷ���                        ��
��                                    �������7-4                       ��                                    ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_insert(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �����������ӽڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ������Ҫ���ӵ�������Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
�ǩ������������������������������������贈�������������������������������贈������������������������������������
��void ngx_rbtree_delete(ngx_rbtree_t ��  tree�Ǻ����������ָ�룻node  ��  �Ӻ������ɾ���ڵ㣬�÷�����      ��
��*tree, ngx_rbtree node_t *node)     ���Ǻ��������Ҫɾ���Ľڵ�ָ��    ��ͨ����ת�������������ƽ��          ��
���������������������������������������ߩ��������������������������������ߩ�������������������������������������
    �ڳ�ʼ�������ʱ����Ҫ�ȷ���ñ���������ngx_rbtree_t�ṹ�壬�Լ�ngx_rbtree_
node_t���͵��ڱ��ڵ㣬��ѡ������Զ���ngx_rbtree_insert_pt���͵Ľڵ����Ӻ�����
    ���ں������ÿ���ڵ���˵�����Ƕ��߱���7-6���е�7�����������ֻ�����˽����
ʹ�ú��������ôֻ��Ҫ�˽�ngx_rbtree_min������


��7��Nginx�ṩ�ĸ߼����ݽṹר233
��7-6������ڵ��ṩ�ķ���
���������������������������������������ש����������������������������������ש���������������������������������������
��    ������                          ��    ��������                      ��    ִ������                          ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_red(node)                   ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    �� t���͵Ľڵ�ָ��                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbt_black(node)                 ��                                  ��  ����node�ڵ����ɫΪ��ɫ            ��
��                                    ��t���͵Ľڵ�ָ��                   ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�O   ��
��ngx_rbt_is_red(node)                ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbt_is_black(node)              ��  node�Ǻ������ngx_rbtree_node_  ��  ��node�ڵ����ɫΪ��ɫ���򷵻ط�0   ��
��                        I           ��t���͵Ľڵ�ָ��                   ����ֵ�����򷵻�O                       ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��               I                    ��  nl��n2���Ǻ������ngx_rbtree_   ��                                      ��
��ngx_rbt_copy_color(nl, n2)          ��                                  ��  ��n2�ڵ����ɫ���Ƶ�nl�ڵ�          ��
��                                 I  ��nodej���͵Ľڵ�ָ��               ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��ngx_rbtree_node_t *                 ��  node�Ǻ������ngx_rbtree_node_  ��                                      ��
��ngx_rbtree_min                      ��t���͵Ľڵ�ָ�룻sentinel����ú� ��  �ҵ���ǰ�ڵ㼰�������е���С�ڵ�    ��
��(ngx_rbtree_node_tľnode,           ���������ڱ��ڵ�                    ��������key�ؼ��֣�                     ��
��ngx_rbtree_node_t *sentinel)        ��                                  ��                                      ��
�ǩ������������������������������������贈���������������������������������贈��������������������������������������
��                                    ��  node�Ǻ������ngx_rbtree_node_  ��  ��ʼ���ڱ��ڵ㣬ʵ���Ͼ��ǽ��ýڵ�  ��
��ngx_rbtree_sentinel_init(node)      ��                                  ��                                      ��
��                                    ��t���͵Ľڵ�ָ��                   ����ɫ��Ϊ��ɫ                          ��
���������������������������������������ߩ����������������������������������ߩ���������������������������������������
    ��7-5�еķ����󲿷�����ʵ�ֻ�����չ������Ĺ��ܣ����ֻ��ʹ�ú��������ôһ
�������ֻ��ʹ��ngx_rbtre e_min������
    ���ڽ��ܵķ������߽ṹ��ļ��÷���ʵ�ֿɲμ�7.5.4�ڵ����ʾ����


ʹ�ú�����ļ�����
    ������һ���򵥵�������˵�����ʹ�ú����������������ջ�з���rbtree���������
�ṹ���Լ��ڱ��ڵ�sentinel����Ȼ��Ҳ����ʹ���ڴ�ػ��ߴӽ��̶��з��䣩�������еĽ�
����ȫ��key�ؼ�����Ϊÿ���ڵ��Ψһ��ʶ�������Ϳ��Բ���Ԥ���ngx_rbtree insert
value�����ˡ����ɵ���ngx_rbtree_init������ʼ�������������������ʾ��
    ngx_rbtree_node_t  sentinel ;
    ngx_rbtree_init ( &rbtree, &sentinel,ngx_str_rbtree_insert_value)
    ���������ڵ�Ľṹ�彫ʹ��7.5.3���н��ܵ�TestRBTreeNode�ṹ�壬���е����н�
�㶼ȡ��ͼ7-7��ÿ��Ԫ�ص�key�ؼ��ְ���1��6��8��11��13��15��17��22��25��27��˳
��һһ�����������ӣ�����������ʾ��
    rbTreeNode [0] .num=17;
    rbTreeNode [1] .num=22;
    rbTreeNode [2] .num=25;
    rbTreeNode [3] .num=27;
    rbTreeNode [4] .num=17;
    rbTreeNode [7] .num=22;
    rbTreeNode [8] .num=25;
    rbTreeNode [9] .num=27;
    for(i=0j i<10; i++)
    {
        rbTreeNode [i].node. key=rbTreeNode[i]. num;
        ngx_rbtree_insert(&rbtree,&rbTreeNode[i].node);
    )
    ������˳��������ĺ������̬��ͼ7-7��ʾ�������Ҫ�ҳ���ǰ���������С�Ľ�
�㣬���Ե���ngx_rbtree_min������ȡ��
ngx_rbtree_node_t *tmpnode   =   ngx_rbtree_min ( rbtree . root ,    &sentinel )  ;
    ��Ȼ�������������ʹ�ø��ڵ����ʹ����һ���ڵ�Ҳ�ǿ��Եġ���������һ�����
����1���ڵ㣬��ȻNginx�Դ˲�û���ṩԤ��ķ����������ַ��������ṩ��ngx_str_
rbtree_lookup��������������ʵ���ϼ����Ƿǳ��򵥵ġ�������Ѱ��key�ؼ���Ϊ13�Ľڵ�
Ϊ��������˵����
    ngx_uint_t lookupkey=13;
    tmpnode=rbtree.root;
    TestRBTreeNode *lookupNode;
    while (tmpnode  !=&sentinel)  {
        if (lookupkey!-tmpnode->key)  (
        ��������key�ؼ����뵱ǰ�ڵ�Ĵ�С�Ƚϣ������Ǽ�������������������
        tmpnode=  (lookupkey<tmpnode->key)  ?tmpnode->left:tmpnode->right;
        continue��
        )
        �����ҵ���ֵΪ13�����ڵ�
        lookupNode=  (TestRBTreeNode*)  tmpnode;
        break;
    )
    �Ӻ������ɾ��1���ڵ�Ҳ�Ƿǳ��򵥵ģ���Ѹո��ҵ���ֵΪ13�Ľڵ��rbtree��
ɾ����ֻ�����ngx_rbtree_delete������
ngx_rbtree_delete ( &rbtree , &lookupNode->node);

7.5.5����Զ������ӳ�Ա����
    ���ڽڵ��key�ؼ��ֱ��������ͣ��⵼�ºܶ�����²�ͬ�Ľڵ�������ͬ��key��
���֡������ϣ�����־�����ͬkey�ؼ��ֵĲ�ͬ�ڵ�������������ʱ���ָ���ԭ�ڵ��
���������Ҫʵ�����е�ngx_rbtree_insert_ptܵ����
    ����Nginxģ����ʹ�ú����ʱ���Զ�����ngx_rbtree_insert_pt��������geo��
filecacheģ��ȣ���������7.5.3���н��ܹ���ngx_str_rbtree insert valueΪ������˵�����





��7��Nginx�ṩ�ĸ߼����ݽṹ��235
���������ķ������ȿ�һ��ngx_str_rbtree insert value��ʵ�֡��������¡�
void :
       I
ngx_str_rbtree_insert_value  ( ngx_rbt ree_node_t    * temp ,
           -l-
     n,gx_rbtree_node_t *node, ngx_rbtree_node t *sentinel)
                                                                                               .
    n:,gx_str_node_t            *n,   ^ t ;
     jgx_rbtree node t :k*p;
l,  n = (ngx_str_node_t *) node;
   I  t = (ngx_str_node t k) temp;
    �������ȱȽ�key�ؼ��֣����������key��Ϊ��һ�����ؼ���
    if  (node- >key!-temp- >key)  {
    �����������ڵ�Ĺؼ���С��������
    p=  (node->key<temp->key)  ?&temp->left  :  &temp->right;
    )
    ������key�ؼ�����ͬʱ�����ַ�������Ϊ�ڶ������ؼ���
    else if  (n- >str. len!_t->str.len)  (
    һ  �����������ڵ��ַ����ĳ���С��������
    p=(n->str. len<t->str. len)  ?&temp- >left:&temp->rightj
    )   else{
    ����key�ؼ�����ͬ���ַ���������ͬʱ���ټ����Ƚ��ַ�������
    p=  (ngx_memcmp (n->str.data,t->str.data,n->str. len)<0)?&temp- >left
&temp - >right j
    )
���������ǰ�ڵ�p���ڱ��ڵ㣬��ô����ѭ��׼������ڵ�
if (*p==sentinel)  {
    break��
)
//p�ڵ���Ҫ����Ľڵ������ͬ�ı�ʶ��ʱ�����븲������
temp=*p;
    *;p=node;
    �����ò���ڵ�ĸ��ڵ�
    node- >parent=temp;
    I�������ӽڵ㶼���ڱ��ڵ�
    node->left=sentinel;
    nOdeһ>righ��=8en��inel��
    ��+���ڵ���ɫ��Ϊ��ɫ��ע�⣬�������ngx-һrbtree insert�������ڿ��ܵ���ת���������øýڵ�
����ɫ+��
    ng��(һrb��һred(node)��
    )  1
    ���Կ������ô�����7.5.4���н��ܹ��ļ����ڵ��������ơ�����Ҫ��������Ҫ��
����ǵ�key�ؼ�����ͬʱ�������Ժ������ݽṹ��Ϊ��׼��ȷ��������ڵ��Ψһ�ԡ�
Nginx���Ѿ�ʵ�ֵ����ngx_rbtree_insert_pt����Ƿǳ����Ƶģ�������ȫ���Բ���ngx_




236��ڶ�������α�дHTTPģ��
str rbtree insert value�������Զ��������ڵ����ӷ�����

*/

void
ngx_rbtree_delete(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{
    ngx_uint_t           red;
    ngx_rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    /* a binary tree delete */

    root = (ngx_rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        temp = node->left;
        subst = node;

    } else {
        subst = ngx_rbtree_min(node->right, sentinel);

        if (subst->left != sentinel) {
            temp = subst->left;
        } else {
            temp = subst->right;
        }
    }

    if (subst == *root) {
        *root = temp;
        ngx_rbt_black(temp);

        /* DEBUG stuff */
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->key = 0;

        return;
    }

    red = ngx_rbt_is_red(subst);

    if (subst == subst->parent->left) {
        subst->parent->left = temp;

    } else {
        subst->parent->right = temp;
    }

    if (subst == node) {

        temp->parent = subst->parent;

    } else {

        if (subst->parent == node) {
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        ngx_rbt_copy_color(subst, node);

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key = 0;

    if (red) {
        return;
    }

    /* a delete fixup */

    while (temp != *root && ngx_rbt_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;

            if (ngx_rbt_is_red(w)) {
                ngx_rbt_black(w);
                ngx_rbt_red(temp->parent);
                ngx_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (ngx_rbt_is_black(w->left) && ngx_rbt_is_black(w->right)) {
                ngx_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ngx_rbt_is_black(w->right)) {
                    ngx_rbt_black(w->left);
                    ngx_rbt_red(w);
                    ngx_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                ngx_rbt_copy_color(w, temp->parent);
                ngx_rbt_black(temp->parent);
                ngx_rbt_black(w->right);
                ngx_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (ngx_rbt_is_red(w)) {
                ngx_rbt_black(w);
                ngx_rbt_red(temp->parent);
                ngx_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (ngx_rbt_is_black(w->left) && ngx_rbt_is_black(w->right)) {
                ngx_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ngx_rbt_is_black(w->left)) {
                    ngx_rbt_black(w->right);
                    ngx_rbt_red(w);
                    ngx_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                ngx_rbt_copy_color(w, temp->parent);
                ngx_rbt_black(temp->parent);
                ngx_rbt_black(w->left);
                ngx_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    ngx_rbt_black(temp);
}


static ngx_inline void
ngx_rbtree_left_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel,
    ngx_rbtree_node_t *node)
{
    ngx_rbtree_node_t  *temp;

    temp = node->right;
    node->right = temp->left;

    if (temp->left != sentinel) {
        temp->left->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->left) {
        node->parent->left = temp;

    } else {
        node->parent->right = temp;
    }

    temp->left = node;
    node->parent = temp;
}


static ngx_inline void
ngx_rbtree_right_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel,
    ngx_rbtree_node_t *node)
{
    ngx_rbtree_node_t  *temp;

    temp = node->left;
    node->left = temp->right;

    if (temp->right != sentinel) {
        temp->right->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->right) {
        node->parent->right = temp;

    } else {
        node->parent->left = temp;
    }

    temp->right = node;
    node->parent = temp;
}