#include <mlpack/core.hpp>
#include <mlpack/methods/random_forest/random_forest.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr std::size_t kFeatureCount = 12;
constexpr std::size_t kClassCount = 2;

struct LoadedDataset
{
    arma::mat features;
    arma::Row<std::size_t> labels;
};

std::vector<std::string> splitCsvLine(const std::string& line)
{
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, ','))
        tokens.push_back(token);

    return tokens;
}

std::size_t parseLabel(const std::string& value)
{
    if (value == "kruskal")
        return 0;
    if (value == "prim")
        return 1;

    throw std::runtime_error("Unknown label in CSV: " + value);
}

LoadedDataset loadDataset(const std::string& csvPath)
{
    std::ifstream input(csvPath);
    if (!input)
        throw std::runtime_error("Cannot open dataset: " + csvPath);

    std::string header;
    if (!std::getline(input, header))
        throw std::runtime_error("Dataset is empty: " + csvPath);

    std::vector<std::array<double, kFeatureCount>> rows;
    std::vector<std::size_t> labels;

    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty())
            continue;

        const std::vector<std::string> tokens = splitCsvLine(line);
        if (tokens.size() != 14)
            throw std::runtime_error("Unexpected CSV column count: " + std::to_string(tokens.size()));

        std::array<double, kFeatureCount> features{};
        for (std::size_t i = 0; i < kFeatureCount; ++i)
            features[i] = std::stod(tokens[i]);

        rows.push_back(features);
        labels.push_back(parseLabel(tokens[12]));
    }

    if (rows.empty())
        throw std::runtime_error("Dataset has no samples: " + csvPath);

    LoadedDataset dataset;
    dataset.features.set_size(kFeatureCount, rows.size());
    dataset.labels.set_size(rows.size());

    for (std::size_t col = 0; col < rows.size(); ++col)
    {
        for (std::size_t row = 0; row < kFeatureCount; ++row)
            dataset.features(row, col) = rows[col][row];
        dataset.labels[col] = labels[col];
    }

    return dataset;
}

void splitDataset(const LoadedDataset& full,
                  arma::mat& trainX,
                  arma::Row<std::size_t>& trainY,
                  arma::mat& testX,
                  arma::Row<std::size_t>& testY,
                  double trainRatio,
                  std::uint32_t seed)
{
    const std::size_t sampleCount = full.features.n_cols;
    std::vector<std::size_t> indices(sampleCount);
    std::iota(indices.begin(), indices.end(), 0);

    std::mt19937 rng(seed);
    std::shuffle(indices.begin(), indices.end(), rng);

    const std::size_t trainCount = std::max<std::size_t>(1, static_cast<std::size_t>(sampleCount * trainRatio));
    const std::size_t testCount = sampleCount - trainCount;
    if (testCount == 0)
        throw std::runtime_error("Need at least one test sample");

    trainX.set_size(full.features.n_rows, trainCount);
    trainY.set_size(trainCount);
    testX.set_size(full.features.n_rows, testCount);
    testY.set_size(testCount);

    for (std::size_t i = 0; i < trainCount; ++i)
    {
        trainX.col(i) = full.features.col(indices[i]);
        trainY[i] = full.labels[indices[i]];
    }

    for (std::size_t i = 0; i < testCount; ++i)
    {
        testX.col(i) = full.features.col(indices[trainCount + i]);
        testY[i] = full.labels[indices[trainCount + i]];
    }
}

double accuracy(const arma::Row<std::size_t>& truth,
                const arma::Row<std::size_t>& pred)
{
    std::size_t correct = 0;
    for (std::size_t i = 0; i < truth.n_elem; ++i)
    {
        if (truth[i] == pred[i])
            ++correct;
    }

    return static_cast<double>(correct) / static_cast<double>(truth.n_elem);
}

arma::Mat<std::size_t> confusionMatrix(const arma::Row<std::size_t>& truth,
                                       const arma::Row<std::size_t>& pred)
{
    arma::Mat<std::size_t> matrix(kClassCount, kClassCount, arma::fill::zeros);
    for (std::size_t i = 0; i < truth.n_elem; ++i)
        ++matrix(truth[i], pred[i]);
    return matrix;
}
} // namespace

int main(int argc, char** argv)
{
    try
    {
        const std::string datasetPath = (argc >= 2) ? argv[1] : "mst_dataset.csv";
        const std::string modelPath = (argc >= 3) ? argv[2] : "mst_rf_model.bin";
        const std::size_t numTrees = (argc >= 4) ? static_cast<std::size_t>(std::stoul(argv[3])) : 100;

        const LoadedDataset dataset = loadDataset(datasetPath);

        arma::mat trainX;
        arma::mat testX;
        arma::Row<std::size_t> trainY;
        arma::Row<std::size_t> testY;
        splitDataset(dataset, trainX, trainY, testX, testY, 0.8, 42);

        mlpack::RandomForest<> forest;
        forest.Train(trainX, trainY, kClassCount, numTrees, 5);

        arma::Row<std::size_t> predictions;
        forest.Classify(testX, predictions);

        const double testAccuracy = accuracy(testY, predictions);
        const arma::Mat<std::size_t> cm = confusionMatrix(testY, predictions);

        if (!mlpack::data::Save(modelPath, "mst_random_forest", forest, true))
            throw std::runtime_error("Failed to save model to " + modelPath);

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Samples: " << dataset.features.n_cols << "\n";
        std::cout << "Train/Test: " << trainX.n_cols << "/" << testX.n_cols << "\n";
        std::cout << "Features: " << dataset.features.n_rows << "\n";
        std::cout << "Trees: " << numTrees << "\n";
        std::cout << "Test accuracy: " << testAccuracy * 100.0 << "%\n";
        std::cout << "Confusion matrix (rows=true, cols=pred; 0=kruskal, 1=prim):\n";
        std::cout << cm(0, 0) << ' ' << cm(0, 1) << "\n";
        std::cout << cm(1, 0) << ' ' << cm(1, 1) << "\n";
        std::cout << "Model saved to " << modelPath << "\n";
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Training failed: " << ex.what() << "\n";
        return 1;
    }
}
