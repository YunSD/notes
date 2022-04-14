# **Mysql trace 工具用例**


----

## 执行

```sql
-- 开启trace
mysql> set session optimizer_trace="enabled=on",end_markers_in_json=on; 

mysql> select * from employees where name > 'a' order by position;
mysql> SELECT * FROM information_schema.OPTIMIZER_TRACE;

output:

```

## 分析

```json

{
    "steps": [
        {
            "join_preparation": { /*第一阶段：SQL准备阶段，格式化sql*/
                "select#": 1,
                "steps": [
                    {
                        "expanded_query": "/* select#1 */ select `employees`.`id` AS `id`,`employees`.`name` AS `name`,`employees`.`age` AS `age`,`employees`.`position` AS `position`,`employees`.`hire_time` AS `hire_time` from `employees` where (`employees`.`name` > 'a') order by `employees`.`position`"
                    }
                ] /* steps */
            } /* join_preparation */
        },
        {
            "join_optimization": { /* 第二阶段：SQL优化阶段 */
                "select#": 1,
                "steps": [
                    {
                        "condition_processing": { /* 条件处理 */
                            "condition": "WHERE",
                            "original_condition": "(`employees`.`name` > 'a')",
                            "steps": [
                                {
                                    "transformation": "equality_propagation",
                                    "resulting_condition": "(`employees`.`name` > 'a')"
                                },
                                {
                                    "transformation": "constant_propagation",
                                    "resulting_condition": "(`employees`.`name` > 'a')"
                                },
                                {
                                    "transformation": "trivial_condition_removal",
                                    "resulting_condition": "(`employees`.`name` > 'a')"
                                }
                            ]  /* steps */
                        }  /* condition_processing */
                    },
                    {
                        "substitute_generated_columns": {} /* substitute_generated_columns */

                    },
                    {
                        "table_dependencies": [ /* 表依赖详情 */
                            {
                                "table": "`employees`",
                                "row_may_be_null": false,
                                "map_bit": 0,
                                "depends_on_map_bits": []  /* depends_on_map_bits */
                            }
                        ] /* table_dependencies */
                    },
                    {
                        "ref_optimizer_key_uses": [] /* ref_optimizer_key_uses */
                    },
                    {
                        "rows_estimation": [ /* 预估表的访问成本 */
                            {
                                "table": "`employees`",
                                "range_analysis": {
                                    "table_scan": { /* 全表扫描情况 */
                                        "rows": 10123, /* 扫描行数 */
                                        "cost": 2054.7 /* 查询成本 */
                                    }, /* table_scan */
                                    "potential_range_indexes": [ /* 查询可能使用的索引 */
                                        {
                                            "index": "PRIMARY", /* 主键索引 */
                                            "usable": false,
                                            "cause": "not_applicable"
                                        },
                                        {
                                            "index": "idx_name_age_position", /* 辅助索引 */
                                            "usable": true,
                                            "key_parts": [
                                                "name",
                                                "age",
                                                "position",
                                                "id"
                                            ] /* key_parts */
                                        }
                                    ], /* potential_range_indexes */
                                    "setup_range_conditions": [], /* setup_range_conditions */
                                    "group_index_range": {
                                        "chosen": false,
                                        "cause": "not_group_by_or_distinct"
                                    }, /* group_index_range */
                                    "analyzing_range_alternatives": { /*分析各个索引使用成本 */
                                        "range_scan_alternatives": [
                                            {
                                                "index": "idx_name_age_position",
                                                "ranges": [
                                                    "a < name" /* 索引使用范围 */
                                                ], /* ranges */
                                                "index_dives_for_eq_ranges": true,
                                                "rowid_ordered": false, /* 使用该索引获取的记录是否按照主键排序 */
                                                "using_mrr": false,
                                                "index_only": false, /* 是否使用覆盖索引 */
                                                "rows": 5061, /* 索引扫描行数 */
                                                "cost": 6074.2, /* 索引使用成本 */
                                                "chosen": false, /* 是否选择该索引 */
                                                "cause": "cost"
                                            }
                                        ], /* range_scan_alternatives */
                                        "analyzing_roworder_intersect": {
                                            "usable": false,
                                            "cause": "too_few_roworder_scans"
                                        } /* analyzing_roworder_intersect */
                                    } /* analyzing_range_alternatives */
                                }  /* range_analysis */
                            }
                        ]  /* rows_estimation */
                    },
                    {
                        "considered_execution_plans": [
                            {
                                "plan_prefix": [], /* plan_prefix */
                                "table": "`employees`",
                                "best_access_path": { /* 最优访问路径 */
                                    "considered_access_paths": [ /* 最终选择的访问路径 */
                                        {
                                            "rows_to_scan": 10123,
                                            "access_type": "scan", /* 访问类型：为scan，全表扫描 */
                                            "resulting_rows": 10123,
                                            "cost": 2052.6,
                                            "chosen": true, /* 确定选择 */
                                            "use_tmp_table": true
                                        }
                                    ] /* considered_access_paths */
                                }, /* best_access_path */
                                "condition_filtering_pct": 100,
                                "rows_for_plan": 10123,
                                "cost_for_plan": 2052.6,
                                "sort_cost": 10123,
                                "new_cost_for_plan": 12176,
                                "chosen": true
                            }
                        ] /* considered_execution_plans */
                    },
                    {
                        "attaching_conditions_to_tables": {
                            "original_condition": "(`employees`.`name` > 'a')",
                            "attached_conditions_computation": [],  /* attached_conditions_computation */
                            "attached_conditions_summary": [
                                {
                                    "table": "`employees`",
                                    "attached": "(`employees`.`name` > 'a')"
                                }
                            ] /* attached_conditions_summary */
                        }  /* attaching_conditions_to_tables */
                    },
                    {
                        "clause_processing": {
                            "clause": "ORDER BY",
                            "original_clause": "`employees`.`position`",
                            "items": [
                                {
                                    "item": "`employees`.`position`"
                                }
                            ],  /* items */
                            "resulting_clause_is_simple": true,
                            "resulting_clause": "`employees`.`position`"
                        }  /* clause_processing */
                    },
                    {
                        "reconsidering_access_paths_for_index_ordering": {
                            "clause": "ORDER BY",
                            "steps": [], /* steps */
                            "index_order_summary": {
                                "table": "`employees`",
                                "index_provides_order": false,
                                "order_direction": "undefined",
                                "index": "unknown",
                                "plan_changed": false
                            } /* index_order_summary */
                        }  /* reconsidering_access_paths_for_index_ordering */
                    },
                    {
                        "refine_plan": [
                            {
                                "table": "`employees`"
                            }
                        ] /* refine_plan */
                    }
                ] /* steps */
            } /* join_optimization */
        },
        {
            "join_execution": { /* 第三阶段：SQL执行阶段 */
                "select#": 1,
                "steps": []  /* steps */
            }  /* join_execution */
        }
    ] /* steps */
}
```

## 结论

**结论：全表扫描的成本低于索引扫描，所以 mysql 最终选择全表扫描。**

```sql 
mysql> select * from employees where name > 'zzz' order by position;
mysql> SELECT * FROM information_schema.OPTIMIZER_TRACE;

-- 查看 trace 字段可知索引扫描的成本低于全表扫描，所以 mysql 最终选择索引扫描

-- 关闭trace
mysql> set session optimizer_trace="enabled=off"; 
```


