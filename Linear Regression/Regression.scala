

object Regression {

  def regression(dataset_file: String,
                 attribute_columns: List[String],
                 value_column: String,
                 test_percentage: Double,
                 alpha: Double,
                 gradient_descent_steps: Int): (Matrix, Double) = {

    val dataset = Dataset(dataset_file).selectColumns(attribute_columns:+value_column)
    val (train_dataset, test_dataset) = dataset.split(test_percentage)
    val train_attr = train_dataset.selectColumns(attribute_columns)
    val train_value = train_dataset.selectColumn(value_column)
    val mat_x = Matrix(train_attr) ++ 1
    val x_height : Double = mat_x.height match {
      case None => 0.0
      case Some(x) => x
    }
    val x_width = mat_x.width match {
      case None => 0
      case Some(x) => x
    }

    val w = Matrix(List.fill(x_width)(List(0.0)))
    val mat_train_value = Matrix(train_value)

    // gradiant descent
    def gradient_descent(acc: Int, mat_w: Matrix): Matrix = {
      if (acc == 0) mat_w
      else {
        val multiplied = mat_x * mat_w
        val Y = multiplied - mat_train_value
        val grad = (mat_x.transpose * Y).map(el => el / x_height)
        val next_w = grad.map(el => el * alpha)

        gradient_descent(acc - 1, mat_w - next_w);
      }
    }
    // evaluare
    val final_w = gradient_descent(gradient_descent_steps, w)
    val test_attr = test_dataset.selectColumns(attribute_columns)
    val test_value = test_dataset.selectColumn(value_column)
    val multi = (Matrix(test_attr) ++ 1) * final_w
    val multi_height: Double = multi.height match
      case Some(h) => h
      case _ => 0
    val error = (multi - Matrix(test_value)).data match
      case Some(x) => x.flatten.foldLeft(0.0)(_+_) / multi_height
      case _ => 0.0
    // evaluare
    (final_w, error)
  }

  def main(args: Array[String]): Unit = {
    print(regression("datasets/houseds.csv", List("GrLivArea", "YearBuilt"), "SalePrice", 0.1, 1e-7, 10000))
  }
}