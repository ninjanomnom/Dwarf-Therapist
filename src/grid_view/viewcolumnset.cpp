/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "columntypes.h"
#include "viewmanager.h"
#include "viewcolumnset.h"
#include "viewcolumnsetdialog.h"
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "spacercolumn.h"
#include "skillcolumn.h"
#include "gamedatareader.h"
#include "defines.h"
#include "labor.h"
#include "utils.h"
#include "dwarftherapist.h"
#include "dwarf.h"
#include "mainwindow.h"
#include "dwarfmodel.h"

ViewColumnSet::ViewColumnSet(QString name, ViewManager *mgr, QObject *parent)
	: QObject(parent)
	, m_name(name)
	, m_manager(mgr)
	, m_bg_color(Qt::white)
{}

void ViewColumnSet::set_name(const QString &name) {
	m_name = name;
}

void ViewColumnSet::add_column(ViewColumn *col) {
	m_columns << col;
}
	
void ViewColumnSet::clear_columns() {
	foreach(ViewColumn *col, m_columns) {
		col->deleteLater();
	}
	m_columns.clear();
}

void ViewColumnSet::update_from_dialog(ViewColumnSetDialog *d) {
	m_name = d->name();
	m_bg_color = d->bg_color();
	clear_columns();
	foreach(ViewColumn *vc, d->columns()) {
		add_column(vc);
	}
}

void ViewColumnSet::toggle_for_dwarf_group() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	QString group_name = a->data().toString();
	DwarfModel *dm = DT->get_main_window()->get_model();

	TRACE << "toggling set:" << name() << "for group:" << group_name;

	int total_enabled = 0;
	int total_labors = 0;
	foreach(Dwarf *d, dm->get_dwarf_groups()->value(group_name)) {
		foreach(ViewColumn *vc, m_columns) {
			if (vc->type() == CT_LABOR) {
				total_labors++;
				LaborColumn *lc = static_cast<LaborColumn*>(vc);
				if (d->is_labor_enabled(lc->labor_id()))
					total_enabled++;
			}
		}
	}
	bool turn_on = total_enabled < total_labors;
	foreach(Dwarf *d, dm->get_dwarf_groups()->value(group_name)) {
		foreach(ViewColumn *vc, m_columns) {
			if (vc->type() == CT_LABOR) {
				LaborColumn *lc = static_cast<LaborColumn*>(vc);
				d->set_labor(lc->labor_id(), turn_on);
			}
		}
	}
	DT->get_main_window()->get_model()->calculate_pending();
}

void ViewColumnSet::toggle_for_dwarf() {
	// find out which dwarf this is for...
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	int dwarf_id = a->data().toInt();
	Dwarf *d = DT->get_dwarf_by_id(dwarf_id);
	toggle_for_dwarf(d);
}

void ViewColumnSet::toggle_for_dwarf(Dwarf *d) {
	TRACE << "toggling set:" << name() << "for" << d->nice_name();
	int total_enabled = 0;
	int total_labors = 0;
	foreach(ViewColumn *vc, m_columns) {
		if (vc->type() == CT_LABOR) {
			total_labors++;
			LaborColumn *lc = static_cast<LaborColumn*>(vc);
			if (d->is_labor_enabled(lc->labor_id()))
				total_enabled++;
		}
	}
	bool turn_on = total_enabled < total_labors;
	foreach(ViewColumn *vc, m_columns) {
		if (vc->type() == CT_LABOR) {
			LaborColumn *lc = static_cast<LaborColumn*>(vc);
			d->set_labor(lc->labor_id(), turn_on);
		}
	}
	DT->get_main_window()->get_model()->calculate_pending();
	
}
