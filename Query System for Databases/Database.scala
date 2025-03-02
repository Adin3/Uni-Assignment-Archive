case class Database(tables: List[Table]) {
  override def toString: String = tables.foldRight("")((xs, x) => x + xs.toString)

  def create(tableName: String): Database = {
    Database(tables :+ Table(tableName, Nil:Tabular))
  }

  def drop(tableName: String): Database = {
    Database(tables.filterNot(table => table.tableName == tableName))
  }

  def selectTables(tableNames: List[String]): Option[Database] = {
    val newTables = tables.foldLeft(Nil: List[Table])((x, xs) => {
      if (tableNames.contains(xs.tableName)) x :+ xs
      else x
    })
    if (newTables.size == tableNames.size) Some(Database(newTables))
    else None
  }
  
  private def verif(s1: String, s2: String): Boolean = {
    val str = s1.split(";").toList
    s1.contains(s2)
  }

  def join(table1: String, c1: String, table2: String, c2: String): Option[Table] = {
    val tb1_temp = tables.filter(t => t.name == table1)
    val tb2_temp = tables.filter(t => t.name == table2)
    if (tb1_temp.isEmpty && tb2_temp.isEmpty) None
    else if(tb1_temp.isEmpty) Some(Table(tb2_temp.head.tableName, tb2_temp.head.tableData))
    else if(tb2_temp.isEmpty) Some(Table(tb1_temp.head.tableName, tb1_temp.head.tableData))
    else {
      val tb1 = tb1_temp.head
      val tb2 = tb2_temp.head
      val tb = tb1.tableData.map(row => {
        val tb2_row = tb2.filter(Field(c2, _ == (row apply c1)))(0)
        if (!tb2_row.isEmpty) tb2_row.foldRight(row)((xs, x) => {
          if (!x.contains(xs._1)) x + xs
          else if (verif(x.apply(xs._1), xs._2)) x
          else x + (xs._1 -> (x.apply(xs._1) + ";" + xs._2))
        }) else Map("null" -> "null")
      }).filterNot(_ == Map("null" -> "null"))
      
      val tbr = tb1.tableData.map(row => {
        val tb2_row = tb2.filter(Field(c2, _ == (row apply c1)))(0)
        if (!tb2_row.isEmpty) Map("null" -> "null") else tb2.header.foldRight(row)((xs, x) => {
          x.get(xs) match {
            case None => x + (xs -> "")
            case Some(v) => x
          }
        })
      }).filterNot(_ == Map("null" -> "null"))
      
      val comtb = tb2.tableData.map(row => {
        val tb1_row = tb1.filter(Field(c1, _ == (row apply c2)))(0)
        if (!tb1_row.isEmpty) Map("null" -> "null") else tb1.header.foldRight(row)((xs, x) => {
          x.get(xs) match {
            case None => x + (xs -> "")
            case Some(v) => x
          }
        })
      }).filterNot(_ == Map("null" -> "null")).map(row => row + (c1 -> (row apply c2)))
      
      val concat = (tb ::: tbr ::: comtb).map(row => if (c1 != c2) row - c2 else row)
      Some(Table(tb2.tableName, concat))
    }
    
  }

  // Implement indexing here
  def size = tables.size

  def apply(i: Int): Table =
    if (size > i) tables(i)
    else Table("", List())
}
