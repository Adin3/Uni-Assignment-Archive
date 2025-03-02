object Queries {

  def killJackSparrow(t: Table): Option[Table] = {
    queryT((Some(t), "FILTER", Field("name", _ != "Jack")))
  }

  def insertLinesThenSort(db: Database): Option[Table] = {
//    val tab:Tabular = List(
//      Map("name"->"Ana", "age"->"93", "CNP"->"455550555"),
//      Map("name"->"Diana", "age"->"33", "CNP"->"255532142"),
//      Map("name"->"Tatiana", "age"->"55", "CNP"->"655532132"),
//      Map("name"->"Rosmaria", "age"->"12", "CNP"->"855532172"),
//    )
    // val tb_fella = queryDB((Some(db), "CREATE", "Inserted Fellas")).get(db.size)
    // val tb_fella1 = queryT(Some(tb_fella), "INSERT", tab)
    queryT(queryT(Some(queryDB((Some(db), "CREATE", "Inserted Fellas")).get(db.size)), "INSERT", List(
      Map("name"->"Ana", "age"->"93", "CNP"->"455550555"),
      Map("name"->"Diana", "age"->"33", "CNP"->"255532142"),
      Map("name"->"Tatiana", "age"->"55", "CNP"->"655532132"),
      Map("name"->"Rosmaria", "age"->"12", "CNP"->"855532172"),
    )), "SORT", "age")
  }

  def youngAdultHobbiesJ(db: Database): Option[Table] = {
//    val any: FilterCond = All(List(
//      Field("hobby", _ != ""),
//      Field("name", _.startsWith("J")),
//      Field("age", _ < "25")
//    ))
//    val tb_join = queryDB(Some(db), "JOIN", "People", "name", "Hobbies", "name").get(0)
//    val filt = queryT(Some(tb_join), "FILTER", any)
    queryT(queryT(Some(queryDB(Some(db), "JOIN", "People", "name", "Hobbies", "name").get(0)), "FILTER", All(List(
      Field("hobby", _ != ""),
      Field("name", _.startsWith("J")),
      Field("age", _ < "25")
    ))), "SELECT", List("name", "hobby"))
  }
}
