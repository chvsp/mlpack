/*

	@file ksinit.cpp

	Tests the working of Kathirvalavakumar Subavathi Initialization

*/


#include <mlpack/core.hpp>

#include <mlpack/methods/ann/activation_functions/logistic_function.hpp>

#include <mlpack/methods/ann/init_rules/random_init.hpp>
#include <mlpack/methods/ann/init_rules/kathirvalavakumar_subavathi_init.hpp>

#include <mlpack/methods/ann/layer/bias_layer.hpp>
#include <mlpack/methods/ann/layer/linear_layer.hpp>
#include <mlpack/methods/ann/layer/base_layer.hpp>
#include <mlpack/methods/ann/layer/binary_classification_layer.hpp>
#include <mlpack/methods/ann/layer/multiclass_classification_layer.hpp>

#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/performance_functions/mse_function.hpp>
#include <mlpack/core/optimizers/rmsprop/rmsprop.hpp>

#include <boost/test/unit_test.hpp>
#include "old_boost_test_definitions.hpp"

using namespace mlpack;
using namespace mlpack::ann;
using namespace mlpack::optimization;

BOOST_AUTO_TEST_SUITE(KSInit);

/**
 * Train and evaluate a vanilla network with the specified structure.
 */
template<
    typename PerformanceFunction,
    typename OutputLayerType,
    typename PerformanceFunctionType,
    typename MatType = arma::mat
>
void BuildVanillaNetwork(MatType& trainData,
                         MatType& trainLabels,
                         MatType& testData,
                         MatType& testLabels,
                         const size_t hiddenLayerSize,
                         const size_t maxEpochs,
                         double& trainError
                         double& testError)
{
  /*
	@param trainError mean squared error of predictions on training data.
 	@param testError  mean squared error of predictions on test data.

   * Construct a feed forward network with trainData.n_rows input nodes,
   * hiddenLayerSize hidden nodes and trainLabels.n_rows output nodes. The
   * network structure looks like:
   *
   *  Input         Hidden        Output
   *  Layer         Layer         Layer
   * +-----+       +-----+       +-----+
   * |     |       |     |       |     |
   * |     +------>|     +------>|     |
   * |     |     +>|     |     +>|     |
   * +-----+     | +--+--+     | +-----+
   *             |             |
   *  Bias       |  Bias       |
   *  Layer      |  Layer      |
   * +-----+     | +-----+     |
   * |     |     | |     |     |
   * |     +-----+ |     +-----+
   * |     |       |     |
   * +-----+       +-----+
   */

  LinearLayer<> inputLayer(trainData.n_rows, hiddenLayerSize);
  BiasLayer<> inputBiasLayer(hiddenLayerSize);
  BaseLayer<PerformanceFunction> inputBaseLayer;

  LinearLayer<> hiddenLayer1(hiddenLayerSize, trainLabels.n_rows);
  BiasLayer<> hiddenBiasLayer1(trainLabels.n_rows);
  BaseLayer<PerformanceFunction> outputLayer;

  OutputLayerType classOutputLayer;

  auto modules = std::tie(inputLayer, inputBiasLayer, inputBaseLayer,
                          hiddenLayer1, hiddenBiasLayer1, outputLayer);

  KathirvalavakumarSubavathiInitialization init(trainData, 4.59);

  FFN<decltype(modules), decltype(classOutputLayer), KathirvalavakumarSubavathiInitialization,
      PerformanceFunctionType> net(modules, classOutputLayer, init);

  RMSprop<decltype(net)> opt(net, 0.01, 0.88, 1e-8,
      maxEpochs * trainData.n_cols, 1e-18);

  net.Train(trainData, trainLabels, opt);

  MatType prediction;
 
  // Calculating the mean squared error on the training data.
  net.Predict(trainData, prediction);
  arma::mat squarederror = arma::pow((prediction*1.0 - trainLabels), 2);
  trainError = arma::sum(arma::sum(squarederror)) / trainData.n_cols;

  // Calculating the mean squared error on the test data
  net.Predict(testData, prediction);
  squarederror = arma::pow((prediction*1.0 - testLabels), 2);
  testError = arma::sum(arma::sum(squarederror)) / testData.n_cols;


}


BOOST_AUTO_TEST_CASE(IrisDataset)
{
   arma::mat trainData, testData, testLabels, trainLabels;

   data::Load("iris_train.csv", trainData, true);
   data::Load("iris_train_labels.csv", trainLabels, true);
   data::Load("iris_test.csv", testData, true);
   data::Load("iris_test_labels.csv", testLabels, true);

   arma::mat biasedTrainData(1, trainData.n_cols);
   arma::mat biasedTestData(1, testData.n_cols);

   biasedTestData.ones();
   biasedTrainData.ones();

   biasedTrainData.insert_rows(1, trainData);
   biasedTestData.insert_rows(1, testData);

   //Running for the 5-3-1 network structure.
    BuildVanillaNetwork<LogisticFunction,
                      MulticlassClassificationLayer,
                      MeanSquaredErrorFunction>
      (biasedTrainData, trainLabels, biasedTestData, testLabels, 3, 62, 0.4);
  


}


BOOST_AUTO_TEST_SUITE_END();









