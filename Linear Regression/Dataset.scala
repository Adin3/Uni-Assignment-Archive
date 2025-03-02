import scala.io.Source

class Dataset(m: List[List[String]]) {
  val data: List[List[String]] = m
  override def toString: String = {
    m.map(ls => ls.foldRight("")((x, y) => x + ',' + y).dropRight(1))
      .foldRight("")((x, y) => x + '\n' + y).dropRight(1)
  }

  private def selectColHelper(col: String): List[String] = {
    m.transpose.map(ls => {
      if (ls.head != col) Nil
      else ls
    }).filter(_.nonEmpty).head
  }

  def selectColumn(col: String): Dataset = {
    val new_m = selectColHelper(col)

    new Dataset(new_m.map(s => List(s)))
  }
  def selectColumns(cols: List[String]): Dataset = {
    val new_m = cols.map(f => selectColHelper(f))

    new Dataset(new_m.transpose)
  }
  def split(percentage: Double): (Dataset, Dataset) = {
    val for_testing =
    if (1 / percentage - (1 / percentage).toInt > 0) (1 / percentage).toInt + 1
    else (1 / percentage).toInt
    val sorted_tail = getRows.sortBy(u => u.head)

    def inner(acc: Int, op: Int => Boolean, list: List[List[String]]): List[List[String]] = {
      if (list.isEmpty) Nil
      else if (op(acc)) list.head :: inner(acc + 1, op, list.tail)
      else inner(acc + 1, op, list.tail)
    }
    val d1 = new Dataset(getHeader::inner(1, (x) => x % for_testing == 0, sorted_tail))
    val d2 = new Dataset(getHeader::inner(1, (x) => x % for_testing != 0, sorted_tail))
    (d2, d1)
  }

  def size = (data.size, data.head.size)
  def getRows: List[List[String]] = data.tail
  def getHeader: List[String] = data.head
}

object Dataset {
  def apply(ds: List[List[String]]): Dataset = {
    new Dataset(ds)
  }
  def apply(csv_filename: String): Dataset = {
    def parseCSV(): List[List[String]] = {
      val iter = Source.fromFile(csv_filename).getLines()
      val listString = iter.foldRight(Nil: List[String])(_ :: _)

      listString.map(ls => ls.split(',').map(_.trim).toList)
    }
    val parse = parseCSV()
    new Dataset(parse)
  }
}
