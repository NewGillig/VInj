void dissect_q931_cause_ie(tvbuff_t *tvb, int offset, int len, proto_tree *tree, int hf_cause_value, guint8 *cause_value, const value_string *ie_vals)
{
    gboolean have_valid_q931_pi_save = have_valid_q931_pi;
    have_valid_q931_pi = FALSE;
    dissect_q931_cause_ie_unsafe(tvb, offset, len, tree, hf_cause_value, cause_value, ie_vals);
    have_valid_q931_pi = have_valid_q931_pi_save;
}