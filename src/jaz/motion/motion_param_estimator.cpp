#include "motion_param_estimator.h"
#include "motion_refiner.h"

#include <src/jaz/index_sort.h>

using namespace gravis;

MotionParamEstimator::MotionParamEstimator(MotionRefiner& motionRefiner)
:   motionRefiner(motionRefiner), ready(false)
{

}

int MotionParamEstimator::read(IOParser& parser, int argc, char *argv[])
{
    parser.addSection("Parameter estimation");

    estim2 = parser.checkOption("--params2", "Estimate 2 parameters instead of motion");
    estim3 = parser.checkOption("--params3", "Estimate 3 parameters instead of motion");
    minParticles = textToInteger(parser.getOption("--min_p", "Minimum number of particles on which to estimate the parameters", "1000"));
    rV = textToFloat(parser.getOption("--r_vel", "Test s_vel +/- r_vel * s_vel", "0.5"));
    rD = textToFloat(parser.getOption("--r_div", "Test s_div +/- r_div * s_div", "0.5"));
    rA = textToFloat(parser.getOption("--r_acc", "Test s_acc +/- r_acc * s_acc", "0.5"));
    recursions = textToInteger(parser.getOption("--par_recs", "Parameter estimation is iterated recursively this many times, each time halving the search range", "3"));
    steps = textToInteger(parser.getOption("--par_steps", "Parameter estimation takes max. this many steps before halving the range", "10"));
    maxRange = textToInteger(parser.getOption("--mot_range", "Limit allowed motion range [Px]", "50"));
}

void MotionParamEstimator::init(const std::vector<MetaDataTable>& allMdts)
{
    if (!motionRefiner.motionEstimator.ready)
    {
        REPORT_ERROR("ERROR: MotionParamEstimator initialized before MotionEstimator.");
    }

    if ((estim2 || estim3) && motionRefiner.motionEstimator.k_cutoff < 0)
    {
        REPORT_ERROR("ERROR: Parameter estimation requires a freq. cutoff (--k_cut / --k_cut_A).");
    }

    if (estim2 && estim3)
    {
        REPORT_ERROR("ERROR: Only 2 or 3 parameters can be estimated (--params2 or --params3), not both.");
    }

    const long mc = allMdts.size();

    std::vector<double> randNums(mc);

    for (int m = 0; m < mc; m++)
    {
        randNums[m] = rand() / (double)RAND_MAX;
    }

    std::vector<int> order = IndexSort<RFLOAT>::sortIndices(randNums);

    int pc = 0;
    mdts.clear();

    for (int i = 0; i < order.size(); i++)
    {
        const int m = order[i];

        const int pcm = allMdts[m].numberOfObjects();

        // motion estimation does not work on one single particle
        if (pcm < 2) continue;

        mdts.push_back(allMdts[m]);
        pc += pcm;

        if (pc > minParticles)
        {
            if (motionRefiner.verb > 0)
            {
                std::cout << " + " << pc << " particles found in "
                          << mdts.size() << " micrographs\n";
            }

            break;
        }
    }

    if (motionRefiner.verb > 0 && pc < minParticles)
    {
        std::cout << " - Warning: this dataset does not contain " << minParticles
                  << " particles in micrographs with at least 2 particles\n";
    }

    ready = true;
}

void MotionParamEstimator::run()
{
    if (!ready)
    {
        REPORT_ERROR("ERROR: MotionParamEstimator::run: MotionParamEstimator not initialized.");
    }



    if (estim2)
    {
        estimateTwoParamsRec();
    }
}

d4Vector MotionParamEstimator::estimateTwoParamsRec()
{

}

void MotionParamEstimator::prepAlignment(
        int k_out,
        const std::vector<Image<RFLOAT>>& dmgWeight0,
        const std::vector<Image<RFLOAT>>& dmgWeight)
{
    /*std::cout << "preparing alignment data...\n";

    std::vector<ParFourierTransformer> fts(nr_omp_threads);

    alignmentSet = AlignmentSet(mdtsAct, fc, s, k_cutoff+2, k_out);

    for (int f = 0; f < fc; f++)
    {
        alignmentSet.damage[f] = alignmentSet.accelerate(dmgWeight0[f]);
    }

    int pctot = 0;

    for (long gg = g0; gg <= gc; gg++)
    {
        const int pc = mdts[gg].numberOfObjects();

        const int g = gg - g0;

        if (pc < 2) continue;

        pctot += pc;

        std::cout << "    micrograph " << (gg+1) << " / " << mdts.size() << ": "
            << pc << " particles [" << pctot << " total]\n";

        std::vector<std::vector<Image<Complex>>> movie;
        std::vector<std::vector<Image<RFLOAT>>> movieCC;

        try
        {
            prepMicrograph(
                gg, fts, dmgWeight,
                movie, movieCC,
                alignmentSet.positions[g],
                alignmentSet.initialTracks[g],
                alignmentSet.globComp[g]);
        }
        catch (RelionError XE)
        {
            std::cerr << "warning: unable to load micrograph #" << (gg+1) << "\n";
            continue;
        }

        Image<float> CCfloat(s,s);

        #pragma omp parallel for num_threads(nr_omp_threads)
        for (int p = 0; p < pc; p++)
        {
            for (int f = 0; f < fc; f++)
            {
                if (maxRange > 0)
                {
                    movieCC[p][f] = FilterHelper::cropCorner2D(movieCC[p][f], 2*maxRange, 2*maxRange);
                }

                for (int y = 0; y < movieCC[p][f]().ydim; y++)
                for (int x = 0; x < movieCC[p][f]().xdim; x++)
                {
                    CCfloat(y,x) = movieCC[p][f](y,x);
                }

                alignmentSet.CCs[g][p][f] = movieCC[p][f];
                alignmentSet.obs[g][p][f] = alignmentSet.accelerate(movie[p][f]);

                Image<Complex> pred;
                int randSubset;
                mdts[gg].getValue(EMDL_PARTICLE_RANDOM_SUBSET, randSubset, p);
                randSubset -= 1;

                if (randSubset == 0)
                {
                    pred = obsModel.predictObservation(projectors[1], mdts[gg], p, true, true);
                }
                else
                {
                    pred = obsModel.predictObservation(projectors[0], mdts[gg], p, true, true);
                }

                alignmentSet.pred[g][p] = alignmentSet.accelerate(pred);
            }
        }
    }

    std::cout << "done\n";*/
}

