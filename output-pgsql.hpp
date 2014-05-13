/* Implements the output-layer processing for osm2pgsql
 * storing the data in several PostgreSQL tables
 * with the final PostGIS geometries for each entity
*/
 
#ifndef OUTPUT_PGSQL_H
#define OUTPUT_PGSQL_H

#include "output.hpp"

#define FLAG_POLYGON 1    /* For polygon table */
#define FLAG_LINEAR  2    /* For lines table */
#define FLAG_NOCACHE 4    /* Optimisation: don't bother remembering this one */
#define FLAG_DELETE  8    /* These tags should be simply deleted on sight */
#define FLAG_PHSTORE 17   /* polygons without own column but listed in hstore this implies FLAG_POLYGON */

/* Table columns, representing key= tags */
struct taginfo {
    char *name;
    char *type;
    int flags;
    int count;
};

struct output_pgsql_t : public output_t {
    output_pgsql_t();
    virtual ~output_pgsql_t();

    int start(const struct output_options *options);
    int connect(const struct output_options *options, int startTransaction);
    void stop();
    void cleanup(void);
    void close(int stopTransaction);

    int node_add(osmid_t id, double lat, double lon, struct keyval *tags);
    int way_add(osmid_t id, osmid_t *nodes, int node_count, struct keyval *tags);
    int relation_add(osmid_t id, struct member *members, int member_count, struct keyval *tags);

    int node_modify(osmid_t id, double lat, double lon, struct keyval *tags);
    int way_modify(osmid_t id, osmid_t *nodes, int node_count, struct keyval *tags);
    int relation_modify(osmid_t id, struct member *members, int member_count, struct keyval *tags);

    int node_delete(osmid_t id);
    int way_delete(osmid_t id);
    int relation_delete(osmid_t id);

    void *pgsql_out_stop_one(void *arg);

private:
    enum table_id {
        t_point, t_line, t_poly, t_roads
    };
    
    struct way_cb_func : public middle_t::way_cb_func {
        output_pgsql_t *m_ptr;
        way_cb_func(output_pgsql_t *ptr);
        virtual ~way_cb_func();
        int operator()(osmid_t id, struct keyval *tags, struct osmNode *nodes, int count, int exists);
    };
    struct rel_cb_func : public middle_t::rel_cb_func  {
        output_pgsql_t *m_ptr;
        rel_cb_func(output_pgsql_t *ptr);
        virtual ~rel_cb_func();
        int operator()(osmid_t id, struct member *, int member_count, struct keyval *rel_tags, int exists);
    };

    friend struct way_cb_func;
    friend struct rel_cb_func;
    
    void write_hstore_columns(enum table_id table, struct keyval *tags);
    void write_wkts(osmid_t id, struct keyval *tags, const char *wkt, enum table_id table);
    int pgsql_out_node(osmid_t id, struct keyval *tags, double node_lat, double node_lon);
    int pgsql_out_way(osmid_t id, struct keyval *tags, struct osmNode *nodes, int count, int exists);
    int pgsql_out_relation(osmid_t id, struct keyval *rel_tags, int member_count, struct osmNode **xnodes, struct keyval *xtags, int *xcount, osmid_t *xid, const char **xrole);
    int pgsql_process_relation(osmid_t id, struct member *members, int member_count, struct keyval *tags, int exists);
    int pgsql_delete_way_from_output(osmid_t osm_id);
    int pgsql_delete_relation_from_output(osmid_t osm_id);

    void pgsql_out_commit(void);
    void copy_to_table(enum table_id table, const char *sql);
    void write_hstore(enum output_pgsql_t::table_id table, struct keyval *tags);


    const struct output_options *m_options;
};

extern output_pgsql_t out_pgsql;

#endif
