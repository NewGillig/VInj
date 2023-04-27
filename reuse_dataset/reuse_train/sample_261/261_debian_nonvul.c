static void dissect_zcl_pwr_prof_pwrprofstatersp(tvbuff_t *tvb, proto_tree *tree, guint *offset)
{
    proto_tree *sub_tree = NULL;
    guint i;
    guint8 power_profile_count;
    power_profile_count = MIN(tvb_get_guint8(tvb, *offset), ZBEE_ZCL_PWR_PROF_NUM_PWR_PROF_ETT);
    proto_tree_add_item(tree, hf_zbee_zcl_pwr_prof_pwr_prof_count, tvb, *offset, 1, ENC_NA);
    *offset += 1;
    for (i = 0; i < power_profile_count; i++)
    {
        sub_tree = proto_tree_add_subtree_format(tree, tvb, *offset, 1, ett_zbee_zcl_pwr_prof_pwrprofiles[i], NULL, "Power Profile #%u", i);
        dissect_zcl_power_profile(tvb, sub_tree, offset);
    }
}