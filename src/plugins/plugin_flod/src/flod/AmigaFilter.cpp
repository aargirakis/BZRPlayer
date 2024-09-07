#include "AmigaFilter.h"
#include "Sample.h"
AmigaFilter::AmigaFilter()
{
    filter = FORCE_OFF;
}
void AmigaFilter::initialize()
{
    l0 = l1 = l2 = l3 = l4 = 0.0;
    r0 = r1 = r2 = r3 = r4 = 0.0;
}
void AmigaFilter::process(Sample* sample)
{
    double d = 1.0 - P0;

    if (model == 0) {
      d = 1 - P0;
      l0 = P0 * sample->l + d * l0 + 1e-18 - 1e-18;
      r0 = P0 * sample->r + d * r0 + 1e-18 - 1e-18;
      d = 1.0 - P1;
      l1 = P1 * l0 + d * l1;
      r1 = P1 * r0 + d * r1;
      sample->l = l1;
      sample->r = r1;
    }

    if ((filter | active) > 0) {
      d = 1.0 - FL;
      l2 = FL * sample->l + d * l2 + 1e-18 - 1e-18;
      r2 = FL * sample->r + d * r2 + 1e-18 - 1e-18;
      l3 = FL * l2 + d * l3;
      r3 = FL * r2 + d * r3;
      l4 = FL * l3 + d * l4;
      r4 = FL * r3 + d * r4;
      sample->l = l4;
      sample->r = r4;
    }

    if (sample->l > 1.0) sample->l = 1.0;
      else if (sample->l < -1.0) sample->l = -1.0;

    if (sample->r > 1.0) sample->r = 1.0;
      else if (sample->r < -1.0) sample->r = -1.0;

}
void AmigaFilter::setModel(int model)
{
    this->model = model;
}
void AmigaFilter::setFilter(int filterVal)
{
    this->filter = filterVal;
}
