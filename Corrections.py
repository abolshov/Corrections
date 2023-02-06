import os
import ROOT
import yaml
import itertools
from RunKit.sh_tools import sh_call
from Common.Utilities import *

from .tau import TauCorrProducer
from .met import METCorrProducer
from .pu import puWeightProducer
from .CorrectionsCore import *

for wpcl in [WorkingPointsTauVSe,WorkingPointsTauVSmu,WorkingPointsTauVSjet]:
    ROOT.gInterpreter.Declare(f'{generate_enum_class(wpcl)}')

initialized = False
tau = None
met = None
pu = None

period_names = {
    'Run2_2016_HIPM': '2016preVFP_UL',
    'Run2_2016': '2016postVFP_UL',
    'Run2_2017': '2017_UL',
    'Run2_2018': '2018_UL',
}

def Initialize(config):
    global initialized
    global tau
    global pu
    global met
    if initialized:
        raise RuntimeError('Corrections are already initialized')
    returncode, output, err= sh_call(['correction', 'config', '--cflags', '--ldflags'],
                                     catch_stdout=True, decode=True, verbose=0)
    params = output.split(' ')
    for param in params:
        if param.startswith('-I'):
            ROOT.gInterpreter.AddIncludePath(param[2:].strip())
        elif param.startswith('-L'):
            lib_path = param[2:].strip()
        elif param.startswith('-l'):
            lib_name = param[2:].strip()
    corr_lib = f"{lib_path}/lib{lib_name}.so"
    if not os.path.exists(corr_lib):
        print(f'correction config output: {output}')
        raise RuntimeError("Correction library is not found.")
    ROOT.gSystem.Load(corr_lib)
    period = config['era']
    pu = puWeightProducer(period=period_names[period])
    tau = TauCorrProducer(period_names[period], config)
    met = METCorrProducer()
    initialized = True

def applyScaleUncertainties(df):
    if not initialized:
        raise RuntimeError('Corrections are not initialized')
    source_dict = {}
    df, source_dict = tau.getES(df, source_dict)
    df, source_dict = met.getPFMET(df, source_dict)
    syst_dict = { }
    for source, source_objs in source_dict.items():
        for scale in getScales(source):
            syst_name = getSystName(source, scale)
            syst_dict[syst_name] = source
            for obj in [ "Electron", "Muon", "Tau", "Jet", "FatJet", "boostedTau", "MET", "PuppiMET",
                         "DeepMETResponseTune", "DeepMETResolutionTune" ]:
                if obj not in source_objs:
                    suffix = 'Central' if obj in [ "Tau", "MET" ] else 'nano'
                    df = df.Define(f'{obj}_p4_{syst_name}', f'{obj}_p4_{suffix}')
    return df,syst_dict


def findRefSample(config, sample_type):
    refSample = []
    for sample, sampleDef in config.items:
        if sampleDef.get('sampleType', None) == sample_type and sampleDef.get('isReference', False):
            refSample.append(sample)
    if len(refSample) != 1:
        raise RuntimeError(f'multiple refSamples for {sample_type}: {refSample}')
    return refSample[0]

def getBranches(syst_name, all_branches):
    final_branches = []
    for branches in all_branches:
        name = syst_name if syst_name in branches else central
        final_branches.extend(branches[name])
    return final_branches

def getNormalisationCorrections(df, config, sample, return_variations=True):
    if not initialized:
        raise RuntimeError('Corrections are not initialized')
    lumi = config['GLOBAL']['luminosity']
    sampleType = config[sample]['sampleType']
    xsFile = config['GLOBAL']['crossSectionsFile']
    xsFilePath = os.path.join(os.environ['ANALYSIS_PATH'], xsFile)
    with open(xsFilePath, 'r') as xs_file:
        xs_dict = yaml.safe_load(xs_file)
    xs_stitching = 1.
    xs_stitching_incl = 1.
    xs_inclusive = 1.
    stitch_str = '1.'
    if sampleType in [ 'DY', 'W']:
        xs_stitching_name = config[sample]['crossSectionStitch']
        inclusive_sample_name = findRefSample(config, sampleType)
        xs_name = config[inclusive_sample_name]['crossSection']
        xs_stitching = xs_dict[xs_stitching_name]['crossSec']
        xs_stitching_incl = xs_dict[config[inclusive_sample_name]['crossSectionStitch']]['crossSec']
        if sampleType == 'DY':
            stitch_str = 'if(LHE_Vpt==0.) return 1/2.; return 1/3.f;'
        elif sampleType == 'W':
            stitch_str= "if(LHE_Njets==0.) return 1.; return 1/2.f;"
    else:
        xs_name = config[sample]['crossSection']

    df = df.Define("stitching_weight", stitch_str)
    xs_inclusive = xs_dict[xs_name]['crossSec']
    stitching_weight_string = f' {xs_stitching} * stitching_weight * ({xs_inclusive}/{xs_stitching_incl})'
    df, pu_SF_branches = pu.getWeight(df)
    df = df.Define('genWeightD', 'std::copysign<double>(1., genWeight)')
    scale = 'Central'
    df, tau_SF_branches = tau.getSF(df, config)
    all_branches = [ pu_SF_branches, tau_SF_branches ]
    all_sources = set(itertools.chain.from_iterable(all_branches))
    all_sources.remove(central)
    all_weights = []
    for syst_name in [central] + list(all_sources):
        branches = getBranches(syst_name, all_branches)
        product = ' * '.join(branches)
        weight_name = f'weight_{syst_name}'
        weight_rel_name = weight_name + '_rel'
        weight_out_name = weight_name if syst_name == central else weight_rel_name
        df = df.Define(weight_name, f'static_cast<float>(genWeightD * {lumi} * {stitching_weight_string} * {product})')
        df = df.Define(weight_rel_name, f'static_cast<float>(weight_{syst_name}/weight_{central})')
        all_weights.append(weight_out_name)
    return df, all_weights

def getDenumerator(df, sources):
    if not initialized:
        raise RuntimeError('Corrections are not initialized')
    df = pu.getWeight(df)
    df = df.Define('genWeightD', 'std::copysign<double>(1., genWeight)')
    syst_names =[]
    for source in sources:
        for scale in getScales(source):
            syst_name = getSystName(source, scale)
            pu_scale = scale if source == pu.uncSource else central
            df = df.Define(f'weight_denum_{syst_name}', f'genWeightD * puWeight_{pu_scale}')
            syst_names.append(syst_name)
    return df,syst_names