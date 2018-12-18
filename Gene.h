//
// Created by Axel on 2018-12-03.
//

#ifndef HUNTERGATHERERS_GENOME_H
#define HUNTERGATHERERS_GENOME_H


#include <map>
#include <list>
#include <memory>
#include <functional>
#include <random>
#include <any>


class Gene {
public:

    explicit Gene(const std::type_info& type);
    const std::type_info& type;

    virtual std::shared_ptr<Gene> Clone() = 0;
    virtual void generate() = 0;
    virtual void mutate(float factor) = 0;
    virtual void evaluate(float mutationFactor, unsigned version) = 0;

    float getMutationWeight() const;
    void setMutationWeight(float mutationWeight);


    Gene* getOwner() const;
    void setOwner(Gene* owner);

    enum State {
        UNEVALUATED,
        EVALUATING,
        EVALUATED
    };

    State getState() const;
    void setState(State state);

    unsigned int getEvaluationCount() const;
    void setEvaluationCount(unsigned int evaluationCount);

    static std::mt19937 randomEngine;

private:
    Gene* owner;

    State state;
    unsigned evaluationCount;
    float mutationWeight;

};

class FloatGene : public Gene {
public:
    FloatGene(float minVal, float maxVal);

    std::shared_ptr<Gene> Clone() override;
    void generate() override;
    void mutate(float factor) override;
    void evaluate(float mutationFactor, unsigned version) override;

    float getValue() const;
    void setValue(float value);

    float getMinVal() const;
    void setMinVal(float minVal);

    float getMaxVal() const;
    void setMaxVal(float maxVal);

private:
    float value;
    float minVal, maxVal;
};


class IntegerGene : public Gene {
public:
    IntegerGene(int minVal, int maxVal);

    std::shared_ptr<Gene> Clone() override;
    void generate() override;
    void mutate(float factor) override;
    void evaluate(float mutationFactor, unsigned version) override;

    int getValue() const;
    void setValue(int value);

    int getMinVal() const;
    void setMinVal(int minVal);

    int getMaxVal() const;
    void setMaxVal(int maxVal);

private:
    int value;
    int minVal, maxVal;
};

class MapGenes : public Gene {
public:
    MapGenes();

    std::shared_ptr<Gene> Clone() override;
    void generate() override;
    void mutate(float factor) override;
    void evaluate(float mutationFactor, unsigned version) override;

    void addGenes(const std::string& name, std::shared_ptr<Gene> gene);
    Gene* getGene(const std::string& name){
        return genes.at(name).get();
    }

    template <class T>
    T* getGene(const std::string& name){
        Gene* g = genes.at(name).get();
        const  std::type_info& c = g->type;
        if (g->type != typeid(T)){
            throw std::runtime_error("Requested gene "+name+" of type "+g->type.name()+", not "+ typeid(T).name()+".");
        }
        return (T*) g;

    }


private:
    std::map<std::string, std::shared_ptr<Gene> > genes;
};

class ListGenes : public Gene {
public:
    ListGenes(std::shared_ptr<Gene> templateGene, std::size_t count);
    ListGenes(std::shared_ptr<Gene> templateGene, std::shared_ptr<IntegerGene> countGene);

    std::shared_ptr<Gene> Clone() override;
    void generate() override;
    void mutate(float factor) override;
    void evaluate(float mutationFactor, unsigned version) override;

    size_t getCount() const;
    void setCount(size_t count);

private:
    std::list<std::shared_ptr<Gene> > genes;
    std::shared_ptr<Gene> templateGene;

    std::size_t count;
    std::shared_ptr<IntegerGene> countGene;
    const bool staticCount;
};


template<class T>
class LambdaGene : public Gene {
public:
    explicit LambdaGene(std::function<T(LambdaGene<T>&, float)> lambda);

    std::shared_ptr<Gene> Clone() override;
    void generate() override;
    void evaluate(float mutationFactor, unsigned version) override;

    T getValue() const;
    void setValue(T value);

private:
    void mutate(float factor) override;
    const std::function<T(LambdaGene&, float)> lambda;

    T value;
};

template class LambdaGene<float>;
template class LambdaGene<int>;
template class LambdaGene<std::string>;
template class LambdaGene<std::any>; // TODO: Does this work?!
template class LambdaGene<void*>;



#endif //HUNTERGATHERERS_GENOME_H
