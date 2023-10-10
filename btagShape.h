#pragma once

#include "corrections.h"
#include "jet.h"

namespace correction {
class bTagShapeCorrProvider : public CorrectionsBase<bTagShapeCorrProvider> {
public:
    enum class UncSource : int {
        Central = -1,
        lf = 0,
        hf = 1,
        lfstats1 = 2,
        lfstats2 = 3,
        hfstats1 = 4,
        hfstats2 = 5,
        cferr1=6,
        cferr2=7,
        jesRelativeBal = 8,
        jesHF = 9,
        jesBBEC1 = 10,
        jesEC2 = 11,
        jesAbsolute = 12,
        jesFlavorQCD = 13,
        jesBBEC1_year = 14,
        jesAbsolute_year = 15,
        jesEC2_year = 16,
        jesHF_year = 17,
        jesRelativeSample_year = 18

    };
    static bool needYear(UncSource source){
        if (source == UncSource::jesBBEC1_year || source==UncSource::jesAbsolute_year ||
        source==UncSource::jesEC2_year || source == UncSource::jesHF_year ||
        source == UncSource::jesRelativeSample_year){return true;}
        return false;
    }
    static const std::map<UncSource,std::string> getUncName (){
        static const std::map<UncSource,std::string> UncMapNames = {
            {UncSource::Central, "Central"},
            {UncSource::lf, "lf"},
            {UncSource::hf, "hf"},
            {UncSource::lfstats1, "lfstats1"},
            {UncSource::lfstats2, "lfstats2"},
            {UncSource::hfstats1, "hfstats1"},
            {UncSource::hfstats2, "hfstats2"},
            {UncSource::cferr1, "cferr1"},
            {UncSource::cferr2, "cferr2"},
            {UncSource::jesRelativeBal, "jesRelativeBal"},
            {UncSource::jesHF, "jesHF"},
            {UncSource::jesBBEC1, "jesBBEC1"},
            {UncSource::jesEC2, "jesEC2"},
            {UncSource::jesAbsolute, "jesAbsolute"},
            {UncSource::jesFlavorQCD, "jesFlavorQCD"},
            {UncSource::jesBBEC1_year, "jesBBEC1_"},
            {UncSource::jesAbsolute_year, "jesAbsolute_"},
            {UncSource::jesEC2_year, "jesEC2_"},
            {UncSource::jesHF_year, "jesHF_"},
            {UncSource::jesRelativeSample_year, "jesRelativeSample_"}
        };
        return UncMapNames;
    }

    static std::string getScaleStr(UncScale scale)
    {
        static const std::map<UncScale, std::string> scale_names = {
            { UncScale::Down, "down_" },
            { UncScale::Up, "up_" },
        };
        if(scale == UncScale::Central)
            return "central";
        return scale_names.at(scale) ;
    }

    static const std::string getFullNameUnc(const std::string scale_name,const std::string source_name, const std::string year, bool need_year, bool isCentral){
        if (isCentral) return "central";
        return need_year ? scale_name+source_name+year : scale_name+source_name;
    }
    //"description": "deepJet reshaping scale factors for UL 2018.
    //The scale factors have 8 default uncertainty sources (hf,lf,hfstats1/2,lfstats1/2,cferr1/2).
    //All except the cferr1/2 uncertainties are to be applied to light and b jets.
    // The cferr1/2 uncertainties are to be applied to c jets.
    //hf/lfstats1/2 uncertainties are to be decorrelated between years, the others correlated.
    //Additional jes-varied scale factors are supplied to be applied for the jes variations.",
    // Note the flavor convention: hadronFlavor is b = 5, c = 4, f = 0


    static bool sourceApplies(UncSource source, int Jet_Flavour, ::correction::JetCorrProvider::UncSource jet_source)
    {
        if(source == UncSource::lf && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Central) return true;
        if(source == UncSource::hf && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Central) return true;
        if(source == UncSource::lfstats1 && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Central) return true;
        if(source == UncSource::lfstats2 && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Central) return true;
        if(source == UncSource::hfstats1 && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Central) return true;
        if(source == UncSource::hfstats2 && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Central) return true;
        if(source == UncSource::cferr1 && Jet_Flavour == 4 && jet_source==::correction::JetCorrProvider::UncSource::Central) return true;
        if(source == UncSource::cferr2 && Jet_Flavour == 4 && jet_source==::correction::JetCorrProvider::UncSource::Central)  return true;

        if (source==UncSource::jesRelativeBal && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::RelativeBal ) return true;
        if (source==UncSource::jesHF && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::HF ) return true;
        if (source==UncSource::jesBBEC1 && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::BBEC1 ) return true;
        if (source==UncSource::jesEC2 && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::EC2 ) return true;
        if (source==UncSource::jesAbsolute && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Absolute ) return true;
        if (source==UncSource::jesFlavorQCD && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::FlavorQCD ) return true;
        if (source==UncSource::jesBBEC1_year && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::BBEC1_year ) return true;
        if (source==UncSource::jesAbsolute_year && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::Absolute_year ) return true;
        if (source==UncSource::jesEC2_year && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::EC2_year ) return true;
        if (source==UncSource::jesHF_year && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::HF_year ) return true;
        if (source==UncSource::jesRelativeSample_year && (Jet_Flavour == 5 || Jet_Flavour==0) && jet_source==::correction::JetCorrProvider::UncSource::RelativeSample_year) return true;
        return false;
    }

    bTagShapeCorrProvider(const std::string& fileName, const std::string& year)  :
    corrections_(CorrectionSet::from_file(fileName)),
    deepJet_shape_(corrections_->at("deepJet_shape")),
    _year(year)
    {
    }


    float getBTagShapeSF(const RVecLV& Jet_p4, const RVecB& pre_sel, const RVecI& Jet_Flavour,const RVecF& Jet_bTag_score, UncSource source, UncScale scale,
    ::correction::JetCorrProvider::UncSource jet_source) const
    {
        double sf_product = 1.;
        for(size_t jet_idx = 0; jet_idx < Jet_p4.size(); jet_idx++){
            if(!pre_sel[jet_idx]) continue;
            const UncScale jet_tag_scale = sourceApplies(source, Jet_Flavour[jet_idx],jet_source)
                                           ? scale : UncScale::Central;
            const std::string& scale_str = getScaleStr(jet_tag_scale);
            bool isCentral = jet_tag_scale == UncScale::Central;
            bool need_year = needYear(source);
            std::string source_str = getUncName().at(source);
            const std::string& unc_name = getFullNameUnc(scale_str, source_str,_year, need_year, isCentral);
            const auto sf = deepJet_shape_->evaluate({unc_name, Jet_Flavour[jet_idx], std::abs(Jet_p4[jet_idx].eta()),Jet_p4[jet_idx].pt(),Jet_bTag_score[jet_idx]});
            sf_product*=sf;
        }

        return sf_product;
    }

private:


private:
    std::unique_ptr<CorrectionSet> corrections_;
    Correction::Ref deepJet_shape_;
    std::string _year;


};

} //namespace correction