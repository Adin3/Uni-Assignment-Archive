type Row = Map[String, String]
type Tabular = List[Row]
// Map.empty[String, String]

case class Table (tableName: String, tableData: Tabular) {

  override def toString: String = {
    val headerCSV = header.foldRight("")((xs, x) => x + xs + ",").dropRight(1) + '\n'
    val tailCSV = data.foldLeft("")((x, xs) => {
      x + header.foldRight("")((el, acc) => acc + xs.apply(el) + ",").dropRight(1) + '\n'
    })
    headerCSV + tailCSV.dropRight(1)
  }

  def insert(row: Row): Table = {
    if (data.exists(currRow => currRow.equals(row))) Table(tableName, tableData)
    else Table(tableName, tableData :+ row)
  }

  def delete(row: Row): Table = {
    Table(tableName, data.filterNot(currRow => currRow.equals(row)))
  }

  def sort(column: String): Table = {
    Table(tableName, data.sortBy(row => row.get(column)))
  }

  def update(f: FilterCond, updates: Map[String, String]): Table = {
    Table(tableName, data.map(row => {
      if (f.eval(row).get) updates.foldRight(row)((xs, x) => {x + xs})
      else row
    }))
  }

  def filter(f: FilterCond): Table = {
    val tb = data.filter(r => {
      f.eval(r) match {
        case None => false
        case Some(x) => x
      }
    })
    val tbFinal = if (tb.isEmpty) List(Map.empty[String, String]) else tb
    Table(tableName, tbFinal)
  }

  def select(columns: List[String]): Table = {
    val listStr = data.map(row => columns.foldLeft(Map.empty[String, String])((x, xs) => x + (xs -> row.apply(xs))))
    Table(tableName, listStr)
  }

  def header: List[String] = data.head.foldRight(Nil: List[String])((xs, x) => x :+ xs._1)
  def data: Tabular = tableData
  def name: String = tableName
}

object Table {
  def apply(name: String, s: String): Table = {
    val str = s.split("\n").map(row => row.split(","))
    val tab = str.tail.map(row => str.head.zip(row).toMap).toList
    Table(name, tab)
  }
}

extension (table: Table) {
  def apply(i: Int): Row =
    if(table.tableData.size > i) table.tableData(i)
    else Map.empty[String, String]// Implement indexing here, find the right function to override
}
