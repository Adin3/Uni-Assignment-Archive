

type Mat = List[List[Double]]

class Matrix(m: Option[Mat]) {

  def transpose: Matrix = {
    m match {
      case Some(x) => {
        def trs(mat: Mat): Mat = {
          if (mat.isEmpty || mat.head.isEmpty) Nil
          else mat.map(ls => ls.head) :: trs(mat.map(ls => ls.tail))
        }
        new Matrix(Some(trs(x)))
      }
      case None => new Matrix(None)
    }
  }
  def map(f: Double => Double): Matrix = {
    m match {
      case Some(x) => {
        val matt = x.map(row => row.map(col => f(col)))
        new Matrix(Some(matt))
      }
      case None => new Matrix(None)
    }
  }
  def *(other: Matrix): Matrix = {
    (m, other.data) match {
      case (_, None) => new Matrix(None)
      case (None, _) => new Matrix(None)
      case (Some(x), Some(y)) => {
        if (x.head.size != y.size) new Matrix(None)
        else {
          val mat = x.map(l => y.transpose.map(ls => l.zip(ls).map(k => k._1 * k._2).foldRight(0.0)(_+_)))
          new Matrix(Some(mat))
        }
      }
    }
  }
  def ++(x: Double): Matrix = {
    m match {
      case Some(y) => {
        new Matrix(Some(y.map(l => l:+x)))
      }
      case None => new Matrix(None)
    }
  }
  def -(other: Matrix): Matrix = {
    (m, other.data) match {
      case (_, None) => new Matrix(None)
      case (None, _) => new Matrix(None)
      case (Some(x), Some(y)) => {
        if (x.head.size != y.head.size || x.size != y.size) new Matrix(None)
        else {
          new Matrix(Some(x.zip(y).map(x => (x._1 zip x._2).map(xx => xx._1 - xx._2))))
        }
      }
    }
  }

  def data: Option[Mat] = m
  def height: Option[Int] = {
    data match {
      case None => None
      case Some(x) => x.size match {
        case 0 => None
        case _ => Some(x.size)
      }
    }
  }
  def width: Option[Int] = {
    data match {
      case None => None
      case Some(x) => x.size match {
        case 0 => None
        case _ => Some(x.head.size)
      }
    }
  }
  override def toString: String = {
    val m_str = m.map(l => l.map(ls => ls.toString()))
    m_str.map(ls => ls.foldRight("")((x, y) => x + "   " + y).dropRight(1))
      .foldRight("")((x, y) => x + '\n' + y).dropRight(1)
  }
}

object Matrix {
  def apply(data: Mat): Matrix = {
    data match {
      case Nil => new Matrix(None)
      case _ => new Matrix(Some(data))
    }
  }
  def apply(data: Option[Mat]): Matrix = {
    new Matrix(data)
  }

  def apply(dataset: Dataset): Matrix = {
    new Matrix(Some(dataset.getRows.map(ls => ls.map(k => k.toDouble))))
  }
}