static bool get_variable_range(PlannerInfo *root, VariableStatData *vardata, Oid sortop, Datum *min, Datum *max)
{
    Datum tmin = 0;
    Datum tmax = 0;
    bool have_data = false;
    int16 typLen;
    bool typByVal;
    Oid opfuncoid;
    Datum *values;
    int nvalues;
    int i;
    return false;
}