#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TCanvas.h"
#include "TH1D.h"
#include "TLatex.h"
#include "Math/Vector4D.h"
#include "TStyle.h"
 
using namespace ROOT;
using namespace ROOT::VecOps;
using RNode = ROOT::RDF::RNode;

using Vec_t = const ROOT::RVec<float>&;
using Vec_i = const ROOT::RVec<int>&;

float ZMass(TLorentzVector tau_p4,TLorentzVector muon_p4) {

    return (tau_p4 + muon_p4).M();
}
float CalcMT(TLorentzVector lep_p4,float met_pt,float met_phi){
  TLorentzVector met_p4;
  met_p4.SetPtEtaPhiE(met_pt,0,met_phi,0);
  const double delta_phi = ROOT::Math::VectorUtil::DeltaPhi(lep_p4, met_p4);
  return std::sqrt( 2.0 * lep_p4.Pt() * met_p4.Pt() * ( 1.0 - std::cos(delta_phi) ) );

}
float deltaR(float eta_1, float eta_2, float phi_1, float phi_2){
   const float deta = eta_1 - eta_2;
   const float dphi = ROOT::Math::VectorUtil::Phi_mpi_pi(phi_1 - phi_2);
   const float dRsq = std::pow(deta,2) + std::pow(dphi,2);

   return sqrt(dRsq);
}
bool PassTagFilter(UInt_t ntrig,Vec_i trig_id,Vec_i trig_bits,Vec_t trig_pt,Vec_t trig_eta,Vec_t trig_phi,float muon_pt,float muon_eta,float muon_phi){
   
   for(int it=0; it < ntrig; it++){
     const ROOT::Math::PtEtaPhiMVector trig(trig_pt[it],trig_eta[it],trig_phi[it],0);
     float dR = deltaR(trig.Eta(),muon_eta,trig.Phi(),muon_phi);
     if (dR < 0.5){
      if((trig_bits[it] & 2) != 0 && trig_id[it] == 13){
           return true;
      }
   }
  }
  return false;
}
int MuonIndex(UInt_t ntrig,Vec_i trig_id,Vec_i trig_bits,Vec_t trig_pt,Vec_t trig_eta,Vec_t trig_phi,UInt_t nMu,Vec_t pt_1, Vec_t eta_1, Vec_t phi_1, Vec_t mass_1,Vec_t pfIso){
  int mu_index = -1;
  if(nMu > 0){
    for(int imu = nMu - 1; imu >= 0; imu--){
      const ROOT::Math::PtEtaPhiMVector muon(pt_1[imu], eta_1[imu], phi_1[imu], mass_1[imu]);
      float mu_iso = pfIso[imu]/muon.Pt();
      if(muon.Pt() < 24)continue;
      if(std::fabs(muon.Eta()) > 2.1)continue;
      if(mu_iso > 0.1)continue;
      if(!PassTagFilter(ntrig,trig_id,trig_bits,trig_pt,trig_eta,trig_phi,muon.Pt(),muon.Eta(),muon.Phi()))continue;
      mu_index = imu;
    }
  }
  return mu_index;
}
int TauIndex(UInt_t ntau, Vec_t pt_1, Vec_t eta_1, Vec_t phi_1, Vec_t mass_1, Vec_t dz_1, TLorentzVector muon_p4){
  int tau_index = -1;
  if(ntau > 0){
    // for (int itau = 0; itau <= ntau; itau++){
      for (int itau = ntau - 1; itau >= 0; itau--){
      const ROOT::Math::PtEtaPhiMVector tau(pt_1[itau], eta_1[itau], phi_1[itau], mass_1[itau]);
      if(dz_1[itau] > 0.2)continue;
      if(deltaR(tau.Eta(), muon_p4.Eta(), tau.Phi(), muon_p4.Phi()) < 0.5) continue;
      if(tau.Pt() < 18) continue;
      if(std::fabs(tau.Eta()) > 2.1) continue; 
      tau_index = itau;
    }  
  }
  return tau_index;
}

// int JetIndex(UInt_t njet, Vec_t pt_1, Vec_t eta_1, Vec_t phi_1, Vec_t mass_1, Vec_t pu_id, Vec_t jet_id, TLorentzVector muon_p4, TLorentzVector tau_p4){
int JetIndex(UInt_t njet, Vec_t pt_1, Vec_t eta_1, Vec_t phi_1, Vec_t mass_1, Vec_t jet_id, TLorentzVector muon_p4, TLorentzVector tau_p4){
  int jet_index = -1;
  if(njet > 0){
    for (int ijet = njet - 1; ijet >= 0; ijet--){
      const ROOT::Math::PtEtaPhiMVector jet(pt_1[ijet], eta_1[ijet], phi_1[ijet], mass_1[ijet]);
      // if ((pu_id[ijet] < 4 && pt_1[ijet] <= 50) || jet_id[ijet] < 2) continue;
      if (jet_id[ijet] < 2) continue;
      if (deltaR(jet.Eta(), muon_p4.Eta(), jet.Phi(), muon_p4.Phi()) < 0.5) continue;
      if (deltaR(jet.Eta(), tau_p4.Eta(), jet.Phi(), tau_p4.Phi()) < 0.5) continue;
      if (jet.Pt() < 18) continue;
      jet_index = ijet;
    }  
  }
  return jet_index;
}

bool PassDiTauFilter(UInt_t ntrig,Vec_i trig_id,Vec_i trig_bits,Vec_t trig_pt,Vec_t trig_eta,Vec_t trig_phi,float tau_pt,float tau_eta,float tau_phi){
   
   for(int it=0; it < ntrig; it++){
     const ROOT::Math::PtEtaPhiMVector trig(trig_pt[it],trig_eta[it],trig_phi[it],0);
     float dR = deltaR(trig.Eta(),tau_eta,trig.Phi(),tau_phi);
     if (dR < 0.5){
       if((trig_bits[it] & 512) != 0 && (trig_bits[it] & 1024) != 0 && trig_id[it] == 15){ 
           return true;
      }
   }
  }
  return false;
}

bool PassDiTauJetFilter(UInt_t ntrig, Vec_i trig_id, Vec_i trig_bits, Vec_t trig_pt, Vec_t trig_eta, Vec_t trig_phi, float jet_pt, float jet_eta, float jet_phi){
  for(int it = 0; it < ntrig; it++) {
    const ROOT::Math::PtEtaPhiMVector trig(trig_pt[it], trig_eta[it], trig_phi[it], 0);
    float dR = deltaR(trig.Eta(), jet_eta, trig.Phi(), jet_phi);
    if (dR < 0.5) {
      // if((trig_bits[it] & 2097152) != 0 && trig_id[it] == 1){ 
      // if(trig_id[it] == 1){ 
        return true;
      // }
    }
  }
  return true; // FIXME when bits are added
}

bool PassMuTauFilter(UInt_t ntrig,Vec_i trig_id,Vec_i trig_bits,Vec_t trig_pt,Vec_t trig_eta,Vec_t trig_phi,float tau_pt,float tau_eta,float tau_phi){
   
   for(int it=0; it < ntrig; it++){
     const ROOT::Math::PtEtaPhiMVector trig(trig_pt[it],trig_eta[it],trig_phi[it],0);
     float dR = deltaR(trig.Eta(),tau_eta,trig.Phi(),tau_phi);
     if (dR < 0.5){
       if((trig_bits[it] & 512) != 0  && trig_id[it] == 15){ 
           return true;
      }
   }
  }
  return false;
}

bool PassElTauFilter(UInt_t ntrig,Vec_i trig_id,Vec_i trig_bits,Vec_t trig_pt,Vec_t trig_eta,Vec_t trig_phi,Vec_t trig_l1pt,Vec_i trig_l1iso,float tau_pt,float tau_eta,float tau_phi){
   
   for(int it=0; it < ntrig; it++){
     const ROOT::Math::PtEtaPhiMVector trig(trig_pt[it],trig_eta[it],trig_phi[it],0);
     float dR = deltaR(trig.Eta(),tau_eta,trig.Phi(),tau_phi);
     if (dR < 0.5){
       if((trig_bits[it] & 512) != 0  && trig_id[it] == 15){ 
	 if(trig_l1pt[it] > 26 && trig_l1iso[it] > 0)
           return true;
      }
   }
  }
  return false;
}

float LeadingTauPT(Vec_t tau_pt,Vec_t tau_eta,Vec_t tau_phi,Vec_t tau_m,int index){
  float pt = 0;
  if(index >= 0){
    const ROOT::Math::PtEtaPhiMVector tau(tau_pt[index],tau_eta[index],tau_phi[index],tau_m[index]);
    pt = tau.Pt();
  }
  return pt;
}

float LeadingTauEta(Vec_t tau_pt,Vec_t tau_eta,Vec_t tau_phi,Vec_t tau_m,int index){
  float eta = -999;
  if(index >= 0){
    const ROOT::Math::PtEtaPhiMVector tau(tau_pt[index],tau_eta[index],tau_phi[index],tau_m[index]);
    eta = tau.Eta();
  }
  return eta;
}
TLorentzVector Obj_p4(int index,Vec_t pt_1, Vec_t eta_1, Vec_t phi_1, Vec_t mass_1){
  TLorentzVector vec_p4(0, 0, 0, 0);
  if(index >= 0){
    const ROOT::Math::PtEtaPhiMVector p4(pt_1[index], eta_1[index], phi_1[index], mass_1[index]);
    vec_p4.SetPtEtaPhiM(p4.Pt(), p4.Eta(), p4.Phi(), p4.M());
  }
  return vec_p4;
}

bool PassBtagVeto(TLorentzVector muon_p4, TLorentzVector tau_p4,UInt_t njet, Vec_t jet_pt, Vec_t jet_eta, Vec_t jet_phi, Vec_t jet_m, Vec_t Jet_btagCSVV2){
  if(njet > 0){
    for(int j = 0; j <= njet; j++){
      const ROOT::Math::PtEtaPhiMVector jet(jet_pt[j], jet_eta[j], jet_phi[j], jet_m[j]);
      if(deltaR(muon_p4.Eta(),jet.Eta(),muon_p4.Phi(),jet.Phi()) > 0.5 && deltaR(tau_p4.Eta(),jet.Eta(),tau_p4.Phi(),jet.Phi()) > 0.5 && jet.Pt() > 20 && std::fabs(jet.Eta()) < 2.4 && Jet_btagCSVV2[j] > 0.0494){
	return true;
      }
    }
  }
  return false;
}
float TauL1_PT(UInt_t ntrig,Vec_i trig_id,Vec_i trig_bits,Vec_t trig_l1pt,Vec_t trig_pt,Vec_t trig_eta,Vec_t trig_phi,float tau_pt,float tau_eta,float tau_phi){
  float taul1_pt = 0;
   for(int it=0; it < ntrig; it++){
     const ROOT::Math::PtEtaPhiMVector trig(trig_pt[it],trig_eta[it],trig_phi[it],0);
     float dR = deltaR(trig.Eta(),tau_eta,trig.Phi(),tau_phi);
     if (dR < 0.5){
       if( trig_id[it] == 15){ 
	 taul1_pt = trig_l1pt[it];
      }
   }
  }
  return taul1_pt;
}
