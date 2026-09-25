/* 学生成绩管理系统 · 精美版交互逻辑 */
(function () {
  'use strict';
  const $ = (s) => document.querySelector(s);
  const body = $('#studentBody');
  const emptyTip = $('#emptyTip');

  // 初始化 ECharts
  const barChart = echarts.init($('#chartBar'));
  const pieChart = echarts.init($('#chartPie'));
  window.addEventListener('resize', () => { barChart.resize(); pieChart.resize(); });

  // 顶部日期
  $('#heroDate').textContent = new Date().toLocaleDateString('zh-CN', {
    year: 'numeric', month: 'long', day: 'numeric', weekday: 'long'
  });

  function toast(msg, type) {
    const t = $('#toast');
    t.textContent = msg;
    t.className = 'toast ' + (type || '');
    setTimeout(() => t.classList.add('hidden'), 2200);
  }
  function errMsg(d, fb) { return (d && d.error) ? d.error : fb; }
  async function api(url, opts) {
    const res = await fetch(url, opts);
    const data = await res.json().catch(() => ({}));
    if (!res.ok) { const e = new Error(errMsg(data, '请求失败')); throw e; }
    return data;
  }
  function esc(s) {
    return String(s).replace(/[&<>"']/g, (c) =>
      ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
  }

  // 渲染表格（rank 按当前列表顺序，默认按平均分降序）
  function renderRows(rows) {
    body.innerHTML = '';
    if (!rows || rows.length === 0) { emptyTip.classList.remove('hidden'); return; }
    emptyTip.classList.add('hidden');
    rows.forEach((s, i) => {
      const rank = i + 1;
      const medalCls = rank === 1 ? 'rank-1' : rank === 2 ? 'rank-2' : rank === 3 ? 'rank-3' : '';
      const medal = rank === 1 ? '🥇' : rank === 2 ? '🥈' : rank === 3 ? '🥉' : '';
      const tr = document.createElement('tr');
      tr.innerHTML =
        `<td><span class="rank-num ${medalCls}">${medal || rank}</span></td>` +
        `<td>${esc(s.id)}</td><td>${esc(s.name)}</td>` +
        `<td>${s.c_score}</td><td>${s.ds_score}</td>` +
        `<td class="avg">${Number(s.average).toFixed(2)}</td>` +
        `<td>
           <button class="btn small" data-act="edit" data-id="${esc(s.id)}">编辑</button>
           <button class="btn small danger" data-act="del" data-id="${esc(s.id)}">删除</button>
         </td>`;
      body.appendChild(tr);
    });
    bindRowActions();
  }

  function renderKpiAndCharts(stats) {
    const d = stats.data;
    if (!d) {
      $('#stTotal').textContent = '0';
      $('#stCAvg').textContent = '-'; $('#stDsAvg').textContent = '-'; $('#stTop').textContent = '-';
      barChart.clear(); pieChart.clear();
      return;
    }
    const c = d.subjects['C语言'], ds = d.subjects['数据结构'];
    $('#stTotal').textContent = d.total;
    $('#stCAvg').textContent = c.avg;
    $('#stDsAvg').textContent = ds.avg;
    $('#stTop').textContent = Math.max(c.max, ds.max);

    // 柱状图：各科 平均/最高/最低
    barChart.setOption({
      tooltip: { trigger: 'axis' },
      legend: { data: ['平均分', '最高分', '最低分'], bottom: 0 },
      grid: { left: 40, right: 20, top: 30, bottom: 50 },
      xAxis: { type: 'category', data: ['C语言', '数据结构'] },
      yAxis: { type: 'value', max: 100 },
      series: [
        { name: '平均分', type: 'bar', data: [c.avg, ds.avg], itemStyle: { color: '#3b82f6' }, barWidth: 26 },
        { name: '最高分', type: 'bar', data: [c.max, ds.max], itemStyle: { color: '#10b981' }, barWidth: 26 },
        { name: '最低分', type: 'bar', data: [c.min, ds.min], itemStyle: { color: '#f59e0b' }, barWidth: 26 }
      ]
    });

    // 环形图：两个科目合计的分数段分布
    const seg = d.segments;
    const labels = ['优(≥90)', '良(80-89)', '中(70-79)', '及格(60-69)', '不及格(<60)'];
    const colors = ['#10b981', '#3b82f6', '#8b5cf6', '#f59e0b', '#ef4444'];
    const data = labels.map((name, i) => ({
      name,
      value: Number(seg['C语言'][i]) + Number(seg['数据结构'][i])
    }));
    pieChart.setOption({
      tooltip: { trigger: 'item', formatter: '{b}: {c} 人 ({d}%)' },
      legend: { bottom: 0 },
      color: colors,
      series: [{
        type: 'pie', radius: ['42%', '68%'], center: ['50%', '45%'],
        avoidLabelOverlap: true,
        itemStyle: { borderRadius: 6, borderColor: '#fff', borderWidth: 2 },
        label: { show: true, formatter: '{b}\n{c}人' },
        data
      }]
    });
  }

  // 默认按平均分降序加载，排名才有意义
  async function loadAll() {
    try {
      const [list, stats] = await Promise.all([
        api('/api/students/sorted?field=average&order=desc'),
        api('/api/stats')
      ]);
      renderRows(list.data);
      $('#listTotal').textContent = `共 ${list.total} 人`;
      renderKpiAndCharts(stats);
    } catch (e) { toast(e.message, 'error'); }
  }

  // 新增
  $('#addForm').addEventListener('submit', async (ev) => {
    ev.preventDefault();
    const f = new FormData(ev.target);
    const payload = {
      id: f.get('id').trim(), name: f.get('name').trim(),
      c_score: Number(f.get('c_score')), ds_score: Number(f.get('ds_score'))
    };
    try {
      const res = await api('/api/students', {
        method: 'POST', headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
      toast(res.message || '录入成功', 'success');
      ev.target.reset();
      await loadAll();
    } catch (e) { toast(e.message, 'error'); }
  });

  // 查询
  $('#searchBtn').addEventListener('click', async () => {
    const q = $('#searchInput').value.trim();
    if (!q) { await loadAll(); return; }
    const type = $('#searchType').value;
    try {
      const res = await api(`/api/students/search?type=${type}&q=${encodeURIComponent(q)}`);
      renderRows(res.data);
      $('#listTotal').textContent = `找到 ${res.total} 人`;
    } catch (e) { toast(e.message, 'error'); }
  });
  $('#searchInput').addEventListener('keydown', (e) => { if (e.key === 'Enter') $('#searchBtn').click(); });
  $('#resetBtn').addEventListener('click', async () => { $('#searchInput').value = ''; await loadAll(); });

  // 排序
  $('#sortBtn').addEventListener('click', async () => {
    const field = $('#sortField').value, order = $('#sortOrder').value;
    try {
      const res = await api(`/api/students/sorted?field=${field}&order=${order}`);
      renderRows(res.data);
      $('#listTotal').textContent = `共 ${res.total} 人`;
    } catch (e) { toast(e.message, 'error'); }
  });

  // 编辑/删除
  function bindRowActions() {
    body.querySelectorAll('button[data-act]').forEach((btn) => {
      btn.addEventListener('click', () => {
        if (btn.dataset.act === 'del') confirmDelete(btn.dataset.id);
        else openEdit(btn.dataset.id);
      });
    });
  }
  async function confirmDelete(id) {
    if (!confirm(`确认删除学号为 ${id} 的学生吗？`)) return;
    try {
      const res = await api(`/api/students/${encodeURIComponent(id)}`, { method: 'DELETE' });
      toast(res.message || '删除成功', 'success');
      await loadAll();
    } catch (e) { toast(e.message, 'error'); }
  }
  async function openEdit(id) {
    try {
      const res = await api(`/api/students/${encodeURIComponent(id)}`);
      const s = res.data;
      $('#editId').textContent = `学号 ${s.id}`;
      $('#editForm').name.value = s.name;
      $('#editForm').c_score.value = s.c_score;
      $('#editForm').ds_score.value = s.ds_score;
      $('#editForm').dataset.id = s.id;
      $('#modal').classList.remove('hidden');
    } catch (e) { toast(e.message, 'error'); }
  }
  $('#editCancel').addEventListener('click', () => $('#modal').classList.add('hidden'));
  $('#editForm').addEventListener('submit', async (ev) => {
    ev.preventDefault();
    const id = ev.target.dataset.id;
    const payload = {
      name: ev.target.name.value.trim(),
      c_score: Number(ev.target.c_score.value),
      ds_score: Number(ev.target.ds_score.value)
    };
    try {
      const res = await api(`/api/students/${encodeURIComponent(id)}`, {
        method: 'PUT', headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
      toast(res.message || '修改成功', 'success');
      $('#modal').classList.add('hidden');
      await loadAll();
    } catch (e) { toast(e.message, 'error'); }
  });
  $('#modal').addEventListener('click', (e) => { if (e.target === $('#modal')) $('#modal').classList.add('hidden'); });

  loadAll();
})();
