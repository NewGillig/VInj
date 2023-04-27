static void *i_alloc_struct_array(gs_memory_t *mem, uint num_elements, gs_memory_type_ptr_t pstype, client_name_t cname)
{
    gs_ref_memory_t *const imem = (gs_ref_memory_t *)mem;
    obj_header_t *obj;
    dmprintf2(mem, " i_alloc_struct_array: called with incorrect structure type (not element), struct='%s', client='%s'\n", pstype->sname, cname);
    return NULL;
}