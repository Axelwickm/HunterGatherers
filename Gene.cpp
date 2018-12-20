//
// Created by Axel on 2018-12-03.
//

#include <time.h>
#include "Gene.h"


std::mt19937 Gene::randomEngine = std::mt19937(static_cast<unsigned int>(time(nullptr)));


Gene::Gene(const std::type_info& type): type(type), state(UNEVALUATED), evaluationCount(0), mutationWeight(1){}


Gene* Gene::getOwner() const {
    return owner;
}

void Gene::setOwner(Gene* owner) {
    Gene::owner = owner;
    evaluationCount = owner->getEvaluationCount();
}


Gene::State Gene::getState() const {
    return state;
}

void Gene::setState(Gene::State state) {
    Gene::state = state;
}


unsigned int Gene::getEvaluationCount() const {
    return evaluationCount;
}

void Gene::setEvaluationCount(unsigned int evaluationCount) {
    //printf("Set evaluation count %d --> %d\n", Gene::evaluationCount, evaluationCount);
    Gene::evaluationCount = evaluationCount;
}

float Gene::getMutationWeight() const {
    return mutationWeight;
}

void Gene::setMutationWeight(float mutationWeight) {
    Gene::mutationWeight = mutationWeight;
}


// Float gene

FloatGene::FloatGene(float minVal, float maxVal): Gene(typeid(FloatGene)), minVal(minVal), maxVal(maxVal) {}

std::shared_ptr<Gene> FloatGene::Clone() {
    FloatGene obj = *this;
    std::shared_ptr<Gene> copy = std::make_shared<FloatGene>(obj);
    return copy;
}

void FloatGene::generate() {
    std::uniform_real_distribution<float> distribution(minVal, maxVal);
    value = distribution(randomEngine);
    setState(EVALUATED);
}

void FloatGene::mutate(float factor) {
    setEvaluationCount(getEvaluationCount()+1);
    float change = factor*getMutationWeight()*(maxVal-minVal);
    std::uniform_real_distribution<float> distribution(-change, change);
    value = std::max(std::min(value + distribution(randomEngine), maxVal), minVal);
}

void FloatGene::evaluate(float mutationFactor, unsigned version) {
    if (getState() == UNEVALUATED){
        setState(EVALUATING);
        generate();
        setState(EVALUATED);
    }
    else if (version != getEvaluationCount()) {
        setState(EVALUATING);
        mutate(mutationFactor);
        setState(EVALUATED);
    }
}

float FloatGene::getValue() const {
    if (getState() != EVALUATED){
        throw std::runtime_error("FloatGene value not evaluated.");
    }
    return value;
}

void FloatGene::setValue(float value) {
    FloatGene::value = value;
}

float FloatGene::getMinVal() const {
    return minVal;
}

void FloatGene::setMinVal(float minVal) {
    FloatGene::minVal = minVal;
}

float FloatGene::getMaxVal() const {
    return maxVal;
}

void FloatGene::setMaxVal(float maxVal) {
    FloatGene::maxVal = maxVal;
}


// Integer gene

IntegerGene::IntegerGene(int minVal, int maxVal) : Gene(typeid(IntegerGene)), minVal(minVal), maxVal(maxVal) {}

std::shared_ptr<Gene> IntegerGene::Clone() {
    IntegerGene obj = *this;
    std::shared_ptr<Gene> copy = std::make_shared<IntegerGene>(obj);
    return copy;
}


void IntegerGene::generate() {
    std::binomial_distribution<int> distribution(maxVal-minVal, 0.5f);
    value = minVal + distribution(randomEngine);
    setState(EVALUATED);
}

void IntegerGene::mutate(float factor) {
    setEvaluationCount(getEvaluationCount()+1);
    // FIXME: this function is mathematically ugly
    int range = ceil((maxVal-minVal)*factor*getMutationWeight());
    std::uniform_int_distribution distribution(-range, range);
    printf("Integer mutate %d ", value);
    value = std::max(std::min(value + distribution(randomEngine), maxVal), minVal);
    printf(" -> %d, factor=%f, eval count = %d\n", value, factor, getEvaluationCount());
}

void IntegerGene::evaluate(float mutationFactor, unsigned version) {
    if (getState() == UNEVALUATED){
        setState(EVALUATING);
        generate();
        setEvaluationCount(version);
    }
    else if (version != getEvaluationCount()) {
        setState(EVALUATING);
        mutate(mutationFactor);
    }
    setState(EVALUATED);
}


int IntegerGene::getValue() const {
    if (getState() != EVALUATED){
        throw std::runtime_error("IntegerGene value not evaluated.");
    }
    return value;
}

void IntegerGene::setValue(int value) {
    IntegerGene::value = value;
}

int IntegerGene::getMinVal() const {
    return minVal;
}

void IntegerGene::setMinVal(int minVal) {
    IntegerGene::minVal = minVal;
}

int IntegerGene::getMaxVal() const {
    return maxVal;
}

void IntegerGene::setMaxVal(int maxVal) {
    IntegerGene::maxVal = maxVal;
}


// Map genes

MapGenes::MapGenes() : Gene(typeid(MapGenes)) {}

std::shared_ptr<Gene> MapGenes::Clone() {
    MapGenes obj = *this;
    obj.genes = std::map<std::string, std::shared_ptr<Gene>>();

    for (auto& gene : genes){
        auto geneCopy = gene.second->Clone();
        geneCopy->setOwner(this);
        genes.emplace(gene.first, std::move(geneCopy));
    }

    std::shared_ptr<Gene> copy = std::make_shared<MapGenes>(obj);
    return copy;
}


void MapGenes::generate() {
    setState(EVALUATING);
    for (auto &gene: genes){
        gene.second->evaluate(0, getEvaluationCount());
    }
    setState(EVALUATED);
}


void MapGenes::mutate(float factor) {
    setEvaluationCount(getEvaluationCount()+1);
    for (auto &gene: genes) {
        //printf("Mutate calling evaluate on %s:\n", gene.first.c_str());
        gene.second->evaluate(factor, getEvaluationCount());
    }
}

void MapGenes::evaluate(float mutationFactor, unsigned version) {
    if (getState() == EVALUATING){
        throw std::runtime_error("MapGene is evaluating (possible loop).");
    }
    unsigned i = getEvaluationCount();
    if (getState() == UNEVALUATED){
        setState(EVALUATING);
        generate();
        setEvaluationCount(version);
    }
    else if (version != getEvaluationCount()) {
        setState(EVALUATING);
        mutate(mutationFactor);
    }
    setState(EVALUATED);
}


void MapGenes::addGenes(const std::string& name, std::shared_ptr<Gene> gene) {
    genes.emplace(name, gene);
    genes.at(name)->setOwner(this);
}


// List Gene

ListGenes::ListGenes(std::shared_ptr<Gene> templateGene, std::size_t count)
: Gene(typeid(ListGenes)), templateGene(std::move(templateGene)), staticCount(true), count(count) {
    ListGenes::templateGene->setOwner(this);
}

ListGenes::ListGenes(std::shared_ptr<Gene> templateGene, std::shared_ptr<IntegerGene> countGene)
: Gene(typeid(ListGenes)), templateGene(std::move(templateGene)), staticCount(false), countGene(std::move(countGene)) {
    ListGenes::templateGene->setOwner(this);
}

std::shared_ptr<Gene> ListGenes::Clone() {
    ListGenes obj = *this;
    obj.genes = std::list<std::shared_ptr<Gene>>();

    for (auto& gene : genes){
        auto geneCopy = gene->Clone();
        geneCopy->setOwner(this);
        obj.genes.push_back(geneCopy);
    }
    std::shared_ptr<Gene> copy = std::make_shared<ListGenes>(obj);
    return copy;
}

void ListGenes::generate() {
    setState(EVALUATING);
    genes.clear();

    if (!staticCount){
        countGene->evaluate(0, getEvaluationCount());
        count = countGene->getValue();
    }

    for (int i = 0; i < count; i++){
        genes.push_back(templateGene->Clone());
    }

    for (auto &gene: genes){
        gene->evaluate(0, getEvaluationCount());
    }
    setState(EVALUATED);
}


void ListGenes::mutate(float factor) {
    setEvaluationCount(getEvaluationCount()+1);

    if (!staticCount){
        countGene->evaluate(factor, getEvaluationCount());
        count = countGene->getValue();
    }

    if (count < genes.size()){
        auto original = genes;
        genes.clear();
        std::sample(original.begin(), original.end(), std::back_inserter(genes), count, randomEngine);
    }
    else if (genes.size() < count){
        auto dist = std::uniform_int_distribution<std::size_t>(0, count-1);
        std::vector<std::size_t> insertPositions;
        insertPositions.reserve(count-genes.size());
        for (std::size_t i = 0; i < count-genes.size(); i++){
            insertPositions.push_back(dist(randomEngine));
        }
        std::sort(insertPositions.begin(), insertPositions.end());

        auto itr = genes.begin();
        std::size_t i = 0;
        for (std::size_t j = 0; j < insertPositions.size(); j++){
            while (insertPositions.at(j)+j != i){
                i++;
                itr = std::next(itr);
            }
            genes.insert(itr, templateGene->Clone());
        }
    }
    int i = 0;
    for (auto &gene: genes) {
        //printf("List calling evaluate on %d:\n", i);
        gene->evaluate(factor, getEvaluationCount());
        i++;
    }
}

void ListGenes::evaluate(float mutationFactor, unsigned version) {
    if (getState() == EVALUATING){
        throw std::runtime_error("ListGene is evaluating (possible loop).");
    }
    if (getState() == UNEVALUATED){
        setState(EVALUATING);
        generate();
        setEvaluationCount(version);
    }
    else if (version != getEvaluationCount()) {
        setState(EVALUATING);
        mutate(mutationFactor);
    }
    setState(EVALUATED);
}

size_t ListGenes::getCount() const {
    return count;
}

void ListGenes::setCount(size_t count) {
    ListGenes::count = count;
}




// Lambda gene

template<class T>
LambdaGene<T>::LambdaGene(std::function<T(LambdaGene<T>&, float)> lambda) : Gene(typeid(LambdaGene)), lambda(lambda) {

}

template <class T>
std::shared_ptr<Gene> LambdaGene<T>::Clone(){
    LambdaGene obj = *this;
    std::shared_ptr<Gene> copy = std::make_shared<LambdaGene>(obj);
    return copy;
}

template<class T>
void LambdaGene<T>::generate() {
    setState(EVALUATING);
    value = lambda(*this, 0.f);
    setState(EVALUATED);
}

template<class T>
void LambdaGene<T>::mutate(float factor) {
    setEvaluationCount(getEvaluationCount()+1);
    setState(EVALUATING);
    value = lambda(*this, factor);
    setState(EVALUATED);
}

template<class T>
void LambdaGene<T>::evaluate(float mutationFactor, unsigned version) {
    if (getState() == EVALUATING){
        throw std::runtime_error("LambdaGene is evaluating (possible loop).");
    }
    if (getState() == UNEVALUATED){
        setState(EVALUATING);
        generate();
        setEvaluationCount(version);
    }
    else if (version != getEvaluationCount()) {
        setState(EVALUATING);
        mutate(mutationFactor);
    }
    setState(EVALUATED);
}


template<class T>
T LambdaGene<T>::getValue() const {
    if (getState() != EVALUATED){
        throw std::runtime_error("LambdaGene value not evaluated.");
    }
    return value;
}

template<class T>
void LambdaGene<T>::setValue(T value) {
    LambdaGene::value = value;
}


