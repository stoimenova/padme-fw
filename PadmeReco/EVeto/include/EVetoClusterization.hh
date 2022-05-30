#ifndef EVetoClustering_h
#define EVetoClustering_h 1
#define MAXHIT 100

#include "EVetoClusterHits.hh"
#include <iostream>

class Cluster{
public:
  Cluster();
  ~Cluster(){};

  int InsertHit(EVetoClusterHits hit, int ihit);
  //  int* GetHitIndex(){return hitIndex;}
  std::vector<int> GetHitIndex(){return hitIndex;}
  int GetMostUpstreamChannel(){return mostUpstreamChannel;}
  int GetMostDownstreamChannel(){return mostDownstreamChannel;}
  double GetAverageTime(){return averagetime;}
  double GetEarlyHitTime(){return earlyhittime;}
  double GetLateHitTime(){return latehittime;}
  int GetNHits(){return nhits;}
  int AddCluster(Cluster* cluster);
  void Print(){std::cout<<"Most Upstream "<<mostUpstreamChannel<<" Most Downstream "<<mostDownstreamChannel<<" average time "<<averagetime<<std::endl;}
  double GetEnergy(){return energy;}
  
  private:
  std::vector<int> hitIndex;
  //  int hitIndex[MAXHIT];
  int mostUpstreamChannel;
  int mostDownstreamChannel;
  double averagetime;
  double earlyhittime;
  double latehittime;
  double energy;
  int nhits;
  
};

class ClusterStructure{
  
public:
  ClusterStructure();
  ~ClusterStructure(){};

  void Clear();
  
  void AddHit(EVetoClusterHits MyHit, int hitindex){    
    HitIndexVec.push_back(hitindex);
    HitVec.push_back(MyHit);
  }

  void HitSort();

  void Clusterise();

  void MergeClusters();

  std::vector<Cluster*> GetClusters(){return ClusVec;}
  std::vector<EVetoClusterHits> GetHitVec(){return HitVec;}
  
private:
  std::vector<EVetoClusterHits> HitVec;
  std::vector<Cluster*> ClusVec;
  std::vector<int> HitIndexVec;
  double ClusterDeltaT=1.5;//maximum time difference between hits in the same cluster
};

#endif
